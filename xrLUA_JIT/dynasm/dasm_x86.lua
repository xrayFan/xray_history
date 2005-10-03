------------------------------------------------------------------------------
-- DynASM x86 module.
--
-- Copyright (C) 2005 Mike Pall. All rights reserved.
-- See dynasm.lua for full copyright notice.
------------------------------------------------------------------------------

-- Module information:
local _info = {
  arch =	"x86",
  description =	"DynASM x86 (i386) module",
  version =	"1.0.2",
  vernum =	 10002,
  release =	"2005-08-29",
  author =	"Mike Pall",
  license =	"MIT",
}

-- Exported glue functions for the arch-specific module.
local _M = { _info = _info }

-- Cache library functions.
local type, tonumber, pairs, ipairs = type, tonumber, pairs, ipairs
local assert = assert
local _s = string
local sub, match, gmatch, gsub = _s.sub, _s.match, _s.gmatch, _s.gsub
local format, byte = _s.format, _s.byte
local concat, sort = table.concat, table.sort

-- Inherited tables and callbacks.
local g_opt, g_arch
local wline, werror, wfatal, wwarn

-- Action name list.
-- CHECK: Keep this in sync with the C code!
local action_names = {
  -- int arg, 1 buffer pos:
  "DISP",  "IMM_S", "IMM_B", "IMM_W", "IMM_D",  "IMM_WB", "IMM_DB",
  -- ptrdiff_t arg, 1 buffer pos: !x64
  "REL_G",
  -- action arg (1 byte) or int arg, 2 buffer pos (link, offset):
  "REL_L", "REL_P",
  -- action arg (1 byte) or int arg, 1 buffer pos (link):
  "IMM_L", "IMM_P",
  -- action arg (1 byte) or int arg, 1 buffer pos (offset):
  "LABEL_L", "LABEL_P",
  -- action arg (1 byte), 1 buffer pos (offset):
  "ALIGN",
  -- action arg (1 byte), no buffer pos.
  "ESC",
  -- no action arg, no buffer pos.
  "MARK",
  -- action arg (1 byte), no buffer pos, terminal action:
  "SECTION",
  -- no args, no buffer pos, terminal action:
  "STOP"
}

-- Maximum number of section buffer positions for dasm_put().
-- CHECK: Keep this in sync with the C code!
local maxsecpos = 15 -- Keep this low, to avoid excessively long C lines.

-- Action name -> action number (dynamically generated below).
local map_action = {}
-- First action number. Everything below does not need to be escaped.
local actfirst = 256-#action_names

-- Action list buffer.
local actlist = {}
-- Argument list for next dasm_put(). Start with offset 0 into action list.
local actargs = { 0 }

-- Current number of section buffer positions for dasm_put().
local secpos = 1

------------------------------------------------------------------------------

-- Compute action numbers for action names.
for n,name in ipairs(action_names) do
  local num = actfirst + n - 1
  map_action[name] = num
end

-- Dump action names and numbers.
local function dumpactions(out)
  out:write("DynASM encoding engine action codes:\n")
  for n,name in ipairs(action_names) do
    local num = map_action[name]
    out:write(format("  %-10s %02X  %d\n", name, num, num))
  end
  out:write("\n")
end

-- Write action list buffer as a huge static C array.
function _M.writelist(out, name)
  local nn = #actlist
  local last = actlist[nn] or 255
  actlist[nn] = nil -- Remove last byte.
  if nn == 0 then nn = 1 end
  out:write("static const unsigned char ", name, "[", nn, "] = {\n")
  local s = "  "
  for n,b in ipairs(actlist) do
    s = s..b..","
    if #s >= 75 then
      assert(out:write(s, "\n"))
      s = "  "
    end
  end
  out:write(s, last, "\n};\n\n") -- Add last byte back.
end

------------------------------------------------------------------------------

-- Add byte to action list.
local function wputxb(n)
  assert(n >= 0 and n <= 255 and n % 1 == 0, "byte out of range")
  actlist[#actlist+1] = n
end

-- Add action to list with optional arg. Advance buffer pos, too.
local function waction(action, a, num)
  wputxb(assert(map_action[action], "bad action name `"..action.."'"))
  if a then actargs[#actargs+1] = a end
  if a or num then secpos = secpos + (num or 1) end
end

-- Add call to embedded DynASM C code.
local function wcall(func, args)
  wline(format("dasm_%s(Dst, %s);", func, concat(args, ", ")), true)
end

-- Delete duplicate action list chunks. Slow, but so what.
local function dedupechunk(offset)
  local al = actlist
  local chunksize = #actlist - offset
  local last = actlist[#actlist]
  for i=0,offset-chunksize do
    if al[i+chunksize] == last then
      local dup = true
      for j=1,chunksize-1 do
	if al[i+j] ~= al[offset+j] then dup = false; break end
      end
      if dup then
	actargs[1] = i -- Replace with original offset.
	for j=offset+1,#actlist do al[j] = nil end -- Kill dupe.
	return
      end
    end
  end
end

-- Flush action list (intervening C code or buffer pos overflow).
local function wflush(term)
  local offset = actargs[1]
  if #actlist == offset then return end -- Nothing to flush.
  if not term then waction("STOP") end -- Terminate action list.
  dedupechunk(offset)
  wcall("put", actargs) -- Add call to dasm_put().
  actargs = { #actlist } -- Actionlist offset is 1st arg to next dasm_put().
  secpos = 1 -- The actionlist offset occupies a buffer position, too.
end

-- Put escaped byte.
local function wputb(n)
  if n >= actfirst then waction("ESC") end -- Need to escape byte.
  wputxb(n)
end

------------------------------------------------------------------------------

-- Arch-specific maps.
local map_archdef = {}		-- Ext. register name -> int. name.
local map_reg_rev = {}		-- Int. register name -> ext. name.
local map_reg_num = {}		-- Int. register name -> register number.
local map_reg_opsize = {}	-- Int. register name -> operand size.
local map_reg_valid_base = {}	-- Int. register name -> valid base register?
local map_reg_valid_index = {}	-- Int. register name -> valid index register?
local reg_list = {}		-- Canonical list of int. register names.

local map_type = {}		-- Type name -> { ctype, reg }
local ctypenum = 0		-- Type number (for _PTx macros).

local addrsize = "d"		-- Size for address operands. !x64

-- Helper function to fill register maps.
local function mkrmap(sz, names)
  for n,name in ipairs(names) do
    local iname = format("@%s%x", sz, n-1)
    reg_list[#reg_list+1] = iname
    map_archdef[name] = iname
    map_reg_rev[iname] = name
    map_reg_num[iname] = n-1
    map_reg_opsize[iname] = sz
    if sz == addrsize then
      map_reg_valid_base[iname] = true
      map_reg_valid_index[iname] = true
    end
  end
  reg_list[#reg_list+1] = ""
end

-- Integer registers (dword, word and byte sized).
mkrmap("d", {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"})
map_reg_valid_index[map_archdef.esp] = nil
mkrmap("w", {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"})
mkrmap("b", {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"})

-- FP registers (internally tword sized, but use "f" as operand size).
mkrmap("f", {"st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7"})

-- Operand size prefixes to codes.
local map_opsize = {
  byte = "b", word = "w", dword = "d", qword = "q", oword = "o", tword = "t",
  aword = addrsize,
}

-- Operand size code to number.
local map_opsizenum = {
  b = 1, w = 2, d = 4, q = 8, o = 16, t = 10,
}

-- Operand size code to name.
local map_opsizename = {
  b = "byte", w = "word", d = "dword", q = "qword", o = "oword", t = "tword",
  f = "fpword",
}

-- Valid index register scale factors.
local map_xsc = {
  ["1"] = 0, ["2"] = 1, ["4"] = 2, ["8"] = 3,
}

-- Condition codes.
local map_cc = {
  o = 0, no = 1, b = 2, nb = 3, e = 4, ne = 5, be = 6, nbe = 7,
  s = 8, ns = 9, p = 10, np = 11, l = 12, nl = 13, le = 14, nle = 15,
  c = 2, nae = 2, nc = 3, ae = 3, z = 4, nz = 5, na = 6, a = 7,
  nge = 12, ge = 13, ng = 14, g = 15,
}


-- Reverse defines for registers.
local function revsubst(s)
  return map_reg_rev[s] or s
end

function _M.revdef(s)
  return gsub(s, "@%w+", revsubst)
end

-- Dump register names and numbers
local function dumpregs(out)
  out:write("Register names, sizes and internal numbers:\n")
  for _,reg in ipairs(reg_list) do
    if reg == "" then
      out:write("\n")
    else
      local name = map_reg_rev[reg]
      local num = map_reg_num[reg]
      local opsize = map_opsizename[map_reg_opsize[reg]]
      out:write(format("  %-5s %-8s %d\n", name, opsize, num))
    end
  end
end

------------------------------------------------------------------------------

-- Put signed byte or arg.
local function wputsbarg(n)
  if type(n) == "number" then
    if n < -128 or n > 127 then
      werror("signed immediate byte out of range")
    end
    if n < 0 then n = n + 256 end
    wputb(n)
  else waction("IMM_S", n) end
end

-- Put unsigned byte or arg.
local function wputbarg(n)
  if type(n) == "number" then
    if n < 0 or n > 255 then
      werror("unsigned immediate byte out of range")
    end
    wputb(n)
  else waction("IMM_B", n) end
end

-- Put unsigned word or arg.
local function wputwarg(n)
  if type(n) == "number" then
    if n < 0 or n > 65535 then
      werror("unsigned immediate word out of range")
    end
    local r = n%256; n = (n-r)/256; wputb(r); wputb(n);
  else waction("IMM_W", n) end
end

-- Put signed or unsigned dword or arg.
local function wputdarg(n)
  if type(n) == "number" then
    if n < 0 then n = n + 4294967296 end
    local r = n%256; n = (n-r)/256; wputb(r);
    r = n%256; n = (n-r)/256; wputb(r);
    r = n%256; n = (n-r)/256; wputb(r); wputb(n);
  else waction("IMM_D", n) end
end

-- Put operand-size dependent number or arg (defaults to dword).
local function wputszarg(sz, n)
  if not sz or sz == "d" then wputdarg(n)
  elseif sz == "w" then wputwarg(n)
  elseif sz == "b" then wputbarg(n)
  elseif sz == "s" then wputsbarg(n)
  else werror("bad operand size") end
end

-- Put action for label arg (IMM_L, IMM_P, REL_L, REL_P).
local function wputlabel(aprefix, imm, num)
  if type(imm) == "number" then
    waction(aprefix.."L", nil, num);
    wputxb(imm < 0 and -imm or 256-imm) -- Reverse sense (fwd < 0).
  else
    waction(aprefix.."P", imm, num)
  end
end

-- Put multi-byte opcode with operand-size dependent modifications.
local function wputop(sz, op)
  local r
  if sz == "w" then wputb(102) end
  if op >= 16777216 then r = op % 16777216 wputb((op-r) / 16777216) op = r end
  if op >= 65536 then r = op % 65536 wputb((op-r) / 65536) op = r end
  if op >= 256 then r = op % 256 wputb((op-r) / 256) op = r end
  if sz == "b" then op = op - 1 elseif sz == "q" then op = op + 4 end
  wputb(op)
end

-- Put ModRM or SIB formatted byte.
local function wputmodrm(m, s, rm)
  assert(m < 4 and s < 8 and rm < 8, "bad modrm operands")
  wputb(64*m + 8*s + rm)
end

-- Put ModRM/SIB plus optional displacement.
local function wputmrmsib(t, s, imark)
  -- Register mode.
  if sub(t.mode, 1, 1) == "r" then
    wputmodrm(3, s, t.reg)
    return
  end

  local disp = t.disp
  -- No base register?
  if not t.reg then
    if t.xreg then
      -- Indexed mode with index register only.
      wputmodrm(0, s, 4) -- [xreg*xsc+disp] -> (0, s, esp) (xsc, xreg, ebp)
      wputmodrm(t.xsc, t.xreg, 5)
    else
      -- Pure displacement.
      wputmodrm(0, s, 5) -- [disp] -> (0, s, ebp)
    end
    wputdarg(disp)
    return
  end

  local m
  if type(disp) == "number" then -- Check displacement size at assembly time.
    if disp == 0 and t.reg ~= 5 then m = 0  -- [ebp] -> [ebp+0] (in SIB, too)
    elseif disp >= -128 and disp <= 127 then m = 1
    else m = 2 end
  end

  -- Index register present or esp as base register: need SIB encoding.
  if t.xreg or t.reg == 4 then
    wputmodrm(m or 2, s, 4) -- ModRM.
    if m == nil or imark then waction("MARK") end
    wputmodrm(t.xsc or 0, t.xreg or 4, t.reg) -- SIB.
  else
    wputmodrm(m or 2, s, t.reg) -- ModRM.
    if imark and (m == 1 or m == 2) then waction("MARK") end
  end

  -- Put displacement.
  if m == 1 then wputsbarg(disp)
  elseif m == 2 then wputdarg(disp)
  elseif not m then waction("DISP", disp) end
end

------------------------------------------------------------------------------

-- Return human-readable operand mode string.
local function opmodestr(op, args)
  local m = {}
  for i=1,#args do
    local a = args[i]
    m[#m+1] = sub(a.mode, 1, 1)..(a.opsize or "?")
  end
  return op.." "..concat(m, ",")
end

-- Convert number to valid integer or nil.
local function toint(expr)
  local n = tonumber(expr)
  if n then
    if n % 1 ~= 0 or n < -2147483648 or n > 4294967295 then
      werror("bad integer number `"..expr.."'")
    end
    return n
  end
end

-- Parse displacement expression: +-num, +-expr, +-opsize*num
local function dispexpr(expr)
  local disp = expr == "" and 0 or toint(expr)
  if not disp then
    local c = sub(expr, 1, 1)
    if c == "+" then
      disp = sub(expr, 2)
    elseif c == '-' then
      disp = expr
    else
      werror("bad displacement expression `"..expr.."'")
    end
    local opsize, tailops = match(expr, "^[+-]%s*(%w+)%s*%*%s*(.+)$")
    local ops, imm = map_opsize[opsize], toint(tailops)
    if ops and imm then
      if c == "-" then imm = -imm end
      disp = imm*map_opsizenum[ops]
    end
  end
  return disp
end

-- Parse register or type expression.
local function rtexpr(expr)
  if not expr then return end
  local tname, ovreg = match(expr, "^([%w_]+):(@[%w_]+)$")
  local tp = map_type[tname or expr]
  if tp then
    local reg = ovreg or tp.reg
    local rnum = map_reg_num[reg]
    if not rnum then
      werror("type `"..(tname or expr).."' needs a register override")
    end
    if not map_reg_valid_base[reg] then
      werror("bad base register override `"..(map_reg_rev[reg] or reg).."'")
    end
    return reg, rnum, tp
  end
  return expr, map_reg_num[expr]
end

-- Parse operand and return { mode, opsize, reg, xreg, xsc, disp, imm }.
local function parseoperand(param)
  local t = {}

  local expr = param
  local opsize, tailops = match(param, "^(%w+)%s*(.+)$")
  if opsize then
    t.opsize = map_opsize[opsize]
    if t.opsize then expr = tailops end
  end

  local br = match(expr, "^%[%s*(.-)%s*%]$")
  repeat
    if br then
      t.mode = "xm"

      -- [disp]
      t.disp = toint(br)
      if t.disp then
	t.mode = "xmO"
	break
      end

      -- [reg...]
      local tp
      local reg, tailr = match(br, "^([@%w_:]+)%s*(.*)$")
      reg, t.reg, tp = rtexpr(reg)
      if not t.reg then
	-- [expr]
	t.mode = "xmO"
	t.disp = br
	break
      end

      -- [xreg*xsc] or [xreg*xsc+-disp] or [xreg*xsc+-expr]
      local xsc, tailsc = match(tailr, "^%*%s*([1248])%s*(.*)$")
      if xsc then
	if not map_reg_valid_index[reg] then
	  werror("bad index register `"..map_reg_rev[reg].."'")
	end
	t.xsc = map_xsc[xsc]
	t.xreg = t.reg
	t.reg = nil
	t.disp = dispexpr(tailsc)
	break
      end
      if not map_reg_valid_base[reg] then
	werror("bad base register `"..map_reg_rev[reg].."'")
      end

      -- [reg] or [reg+-disp]
      t.disp = toint(tailr) or (tailr == "" and 0)
      if t.disp then break end

      -- [reg+xreg...]
      local xreg, tailx = match(tailr, "^+%s*([@%w_:]+)%s*(.*)$")
      xreg, t.xreg, tp = rtexpr(xreg)
      if not t.xreg then
	-- [reg+-expr]
	t.disp = dispexpr(tailr)
	break
      end
      if not map_reg_valid_index[xreg] then
	werror("bad index register `"..map_reg_rev[xreg].."'")
      end

      -- [reg+xreg*xsc...]
      local xsc, tailsc = match(tailx, "^%*%s*([1248])%s*(.*)$")
      if xsc then
	t.xsc = map_xsc[xsc]
	tailx = tailsc
      end

      -- [...] or [...+-disp] or [...+-expr]
      t.disp = dispexpr(tailx)
    else
      -- imm or opsize*imm
      local imm = toint(expr)
      if not imm and sub(expr, 1, 1) == "*" and t.opsize then
	imm = toint(sub(expr, 2))
	if imm then
	  imm = imm * map_opsizenum[t.opsize]
	  t.opsize = nil
	end
      end
      if imm then
	if t.opsize then werror("bad operand size override") end
	t.imm = imm
	local m = "i"
	if imm == 1 then m = m.."1" end
	if imm >= -128 and imm <= 127 then m = m.."S" end
	t.mode = m
	break
      end

      local tp
      local reg, tailr = match(expr, "^([@%w_:]+)%s*(.*)$")
      reg, t.reg, tp = rtexpr(reg)
      if t.reg then
	-- reg
	if tailr == "" then
	  if t.opsize then werror("bad operand size override") end
	  t.opsize = map_reg_opsize[reg]
	  if t.opsize == "f" then
	    t.mode = t.reg == 0 and "fF" or "f"
	  else
	    if reg == "@w4" then wwarn("bad idea, try again with `esp'") end
	    t.mode = t.reg == 0 and "rmR" or (reg == "@b1" and "rmC" or "rm")
	  end
	  break
	end

	-- type[idx], type[idx].field, type->field -> [reg+offset_expr]
	if not tp then werror("bad operand `"..param.."'") end
	t.mode = "xm"
	t.disp = format(tp.ctypefmt, tailr)
      else
	-- &expr (pointer)
	if sub(expr, 1, 1) == "&" then
	  t.imm = format("(ptrdiff_t)(%s)", sub(expr,2))
	  t.opsize = addrsize
	  t.mode = "iPJ"
	  break
	end

	-- ->expr (pc label reference)
	if sub(expr, 1, 2) == "->" then
	  t.imm = sub(expr, 3)
	  t.opsize = addrsize
	  t.mode = "iJ"
	  break
	end

	-- [<>][1-9] (local label reference)
	local dir, lnum = match(expr, "^([<>])([1-9])$")
	if dir then
	  t.imm = lnum * (dir == ">" and 1 or -1)
	  t.opsize = addrsize
	  t.mode = "iJ"
	  break
	end

	-- expr (interpreted as immediate)
	t.mode = "iI"
	t.imm = expr
      end
    end
  until true
  return t
end

------------------------------------------------------------------------------
-- x86 Template String Description
-- ===============================
--
-- Each template string is a list of [match:]pattern pairs,
-- separated by "|". The first match wins. No match means a
-- bad or unsupported combination of operand modes or sizes.
--
-- The match part and the ":" is omitted if the operation has
-- no operands. Otherwise the first N characters are matched
-- against the mode strings of each of the N operands.
--
-- The mode string for each operand type is (see parseoperand()):
--   Integer register: "rm", +"R" for eax, ax, al, +"C" for cl
--   FP register:      "f",  +"F" for st0
--   Index operand:    "xm", +"O" for [disp] (pure offset)
--   Immediate:        "i",  +"S" for signed 8 bit, +"1" for 1,
--                     +"I" for arg, +"P" for pointer
--   Any:              +"J" for valid jump targets
--
-- So a match character "m" (mixed) matches both an integer register
-- and an index operand (to be encoded with the ModRM/SIB scheme).
-- But "r" matches only a register and "x" only an index operand
-- (e.g. for FP memory access operations).
--
-- The operand size match string starts right after the mode match
-- characters and ends before the ":". "dwb" is assumed, if empty.
-- The effective data size of the operation is matched against this list.
--
-- If only the regular "b", "w", "d", "q", "t" operand sizes are
-- present, then all operands must be the same size. Unspecified sizes
-- are ignored, but at least one operand must have a size or the pattern
-- won't match (use the "byte", "word", "dword", "qword", "tword"
-- operand size overrides. E.g.: mov dword [eax], 1).
--
-- If the list has a "1" or "2" prefix, the operand size is taken
-- from the respective operand and any other operand sizes are ignored.
-- If the list contains only ".", all operand sizes are ignored.
--
-- E.g. "rrdw" matches for either two dword registers or two word
-- registers. "Fx2dq" matches an st0 operand plus an index operand
-- pointing to a dword (float) or qword (double).
--
-- Every character after the ":" is part of the pattern string:
--   Hex chars are accumulated to form the opcode (left to right).
--   "n"       disables the standard opcode mods (otherwise: -1 for "b",
--             o16 prefix for "w" and +4 for "q")
--   "r"/"R"   adds the reg. number from the 1st/2nd operand to the opcode.
--   "m"/"M"   generates ModRM/SIB from the 1st/2nd operand.
--             The spare 3 bits are either filled with the last hex digit or
--             the result from a previous "r"/"R". The opcode is restored.
--
-- All of the following characters force a flush of the opcode:
--   "o"/"O"   stores a pure 32 bit disp (offset) from the 1st/2nd operand.
--   "S"       stores a signed 8 bit immediate from the last operand.
--   "U"       stores an unsigned 8 bit immediate from the last operand.
--   "W"       stores an unsigned 16 bit immediate from the last operand.
--   "i"       stores an operand sized immediate from the last operand.
--   "I"       dito, but generates an action code to optionally modify
--             the opcode (+2) for a signed 8 bit immediate.
--   "J"       generates one of the REL action codes from the last operand.
--
------------------------------------------------------------------------------

-- Template strings for x86 instructions. Ordered by first opcode byte.
-- Unimplemented opcodes (deliberate omissions) are marked with *.
local map_op = {
  -- 00-05: add...
  -- 06: *push es
  -- 07: *pop es
  -- 08-0D: or...
  -- 0E: *push cs
  -- 0F: two byte opcode prefix
  -- 10-15: adc...
  -- 16: *push ss
  -- 17: *pop ss
  -- 18-1D: sbb...
  -- 1E: *push ds
  -- 1F: *pop ds
  -- 20-25: and...
  es_0 =	"26",
  -- 27: *daa
  -- 28-2D: sub...
  cs_0 =	"2E",
  -- 2F: *das
  -- 30-35: xor...
  ss_0 =	"36",
  -- 37: *aaa
  -- 38-3D: cmp...
  ds_0 =	"3E",
  -- 3F: *aas
  inc_1 =	"rdw:40r|m:FF0m",
  dec_1 =	"rdw:48r|m:FF1m",
  push_1 =	"rdw:50r|mdw:FF6m|S.:6AS|ib:n6Ai|i.:68i",
  pop_1 =	"rdw:58r|mdw:8F0m",
  -- 60: *pusha, *pushad, *pushaw
  -- 61: *popa, *popad, *popaw
  -- 62: *bound rdw,x
  -- 63: *arpl mw,rw
  fs_0 =	"64",
  gs_0 =	"65",
  o16_0 =	"66",
  a16_0 =	"67",
  -- 68: push idw
  -- 69: imul rdw,mdw,idw
  -- 6A: push ib
  -- 6B: imul rdw,mdw,S
  -- 6C: *insb
  -- 6D: *insd, *insw
  -- 6E: *outsb
  -- 6F: *outsd, *outsw
  -- 70-7F: jcc lb
  -- 80: add... mb,i
  -- 81: add... mdw,i
  -- 82: *undefined
  -- 83: add... mdw,S
  test_2 =	"mr:85Rm|rm:85rM|Ri:A9i|mi:F70mi",
  -- 86: xchg rb,mb
  -- 87: xchg rdw,mdw
  -- 88: mov mb,r
  -- 89: mov mdw,r
  -- 8A: mov r,mb
  -- 8B: mov r,mdw
  -- 8C: *mov mdw,seg
  lea_2 =	"rxd:8DrM",
  -- 8E: *mov seg,mdw
  -- 8F: pop mdw
  nop_0 =	"90",
  xchg_2 =	"Rrdw:90R|rRdw:90r|rm:87rM|mr:87Rm",
  cbw_0 =	"6698",
  cwde_0 =	"98",
  cwd_0 =	"6699",
  cdq_0 =	"99",
  -- 9A: *call iw:idw
  wait_0 =	"9B",
  fwait_0 =	"9B",
  pushf_0 =	"9C",
  pushfw_0 =	"669C",
  pushfd_0 =	"9C",
  popf_0 =	"9D",
  popfw_0 =	"669D",
  popfd_0 =	"9D",
  sahf_0 =	"9E",
  lahf_0 =	"9F",
  mov_2 =	"OR:A3o|RO:A1O|mr:89Rm|rm:8BrM|rib:nB0ri|ridw:B8ri|mi:C70mi",
  movsb_0 =	"A4",
  movsw_0 =	"66A5",
  movsd_0 =	"A5",
  cmpsb_0 =	"A6",
  cmpsw_0 =	"66A7",
  cmpsd_0 =	"A7",
  -- A8: test Rb,i
  -- A9: test Rdw,i
  stosb_0 =	"AA",
  stosw_0 =	"66AB",
  stosd_0 =	"AB",
  lodsb_0 =	"AC",
  lodsw_0 =	"66AD",
  lodsd_0 =	"AD",
  scasb_0 =	"AE",
  scasw_0 =	"66AF",
  scasd_0 =	"AF",
  -- B0-B7: mov rb,i
  -- B8-BF: mov rdw,i
  -- C0: rol... mb,i
  -- C1: rol... mdw,i
  ret_1 =	"i.:nC2W",
  ret_0 =	"C3",
  -- C4: *les rdw,mq
  -- C5: *lds rdw,mq
  -- C6: mov mb,i
  -- C7: mov mdw,i
  -- C8: *enter iw,ib
  leave_0 =	"C9",
  -- CA: *retf iw
  -- CB: *retf
  int3_0 =	"CC",
  int_1 =	"i.:nCDU",
  into_0 =	"CE",
  -- CF: *iret
  -- D0: rol... mb,1
  -- D1: rol... mdw,1
  -- D2: rol... mb,cl
  -- D3: rol... mb,cl
  -- D4: *aam ib
  -- D5: *aad ib
  -- D6: *salc
  -- D7: *xlat
  -- D8-DF: floating point ops
  -- E0: *loopne
  -- E1: *loope
  -- E2: *loop
  -- E3: *jcxz, *jecxz
  -- E4: *in Rb,ib
  -- E5: *in Rdw,ib
  -- E6: *out ib,Rb
  -- E7: *out ib,Rdw
  call_1 =	"md:FF2m|J.:E8J",
  jmp_1 =	"md:FF4m|J.:E9J", -- short: EB
  -- EA: *jmp iw:idw
  -- EB: jmp ib
  -- EC: *in Rb,dx
  -- ED: *in Rdw,dx
  -- EE: *out dx,Rb
  -- EF: *out dx,Rdw
  -- F0: *lock
  int1_0 =	"F1",
  repne_0 =	"F2",
  repnz_0 =	"F2",
  rep_0 =	"F3",
  repe_0 =	"F3",
  repz_0 =	"F3",
  -- F4: *hlt
  cmc_0 =	"F5",
  -- F6: test... mb,i; div... mb
  -- F7: test... mdw,i; div... mdw
  clc_0 =	"F8",
  stc_0 =	"F9",
  -- FA: *cli
  cld_0 =	"FC",
  std_0 =	"FD",
  -- FE: inc... mb
  -- FF: inc... mdw

  -- misc ops
  not_1 =	"m:F72m",
  neg_1 =	"m:F73m",
  mul_1 =	"m:F74m",
  imul_1 =	"m:F75m",
  div_1 =	"m:F76m",
  idiv_1 =	"m:F77m",

  imul_2 =	"rmdw:0FAFrM|rIdw:69rmI|rSdw:6BrmS|ridw:69rmi",
  imul_3 =	"rmIdw:69rMI|rmSdw:6BrMS|rmidw:69rMi",

  rdtsc_0 =	"0F31", -- P1+
  cpuid_0 =	"0FA2", -- P1+

  -- floating point ops
  fst_1 =	"ff:DDD0r|xdq:D92m",
  fstp_1 =	"ff:DDD8r|xdq:D93m|xt:DB7m",
  fld_1 =	"ff:D9C0r|xdq:D90m|xt:DB5m",

  fpop_0 =	"DDD8", -- Alias for fstp st0.

  fist_1 =	"xw:nDF2m|xd:DB2m",
  fistp_1 =	"xw:nDF3m|xd:DB3m|xq:nDF7m",
  fild_1 =	"xw:nDF0m|xd:DB0m|xq:nDF5m",

  fxch_0 =	"D9C9",
  fxch_1 =	"ff:D9C8r",
  fxch_2 =	"fFf:D9C8r|Fff:D9C8R",

  fucom_1 =	"ff:DDE0r",
  fucom_2 =	"Fff:DDE0R",
  fucomp_1 =	"ff:DDE8r",
  fucomp_2 =	"Fff:DDE8R",
  fucomi_1 =	"ff:DBE8r", -- P6+
  fucomi_2 =	"Fff:DBE8R", -- P6+
  fucomip_1 =	"ff:DFE8r", -- P6+
  fucomip_2 =	"Fff:DFE8R", -- P6+
  fcomi_1 =	"ff:DBF0r", -- P6+
  fcomi_2 =	"Fff:DBF0R", -- P6+
  fcomip_1 =	"ff:DFF0r", -- P6+
  fcomip_2 =	"Fff:DFF0R", -- P6+
  fucompp_0 =	"DAE9",
  fcompp_0 =	"DED9",

  fldcw_1 =	"xw:nD95m",
  fstcw_1 =	"xw:n9BD97m",
  fnstcw_1 =	"xw:nD97m",
  fstsw_1 =	"Rw:n9BDFE0|xw:n9BDD7m",
  fnstsw_1 =	"Rw:nDFE0|xw:nDD7m",
  fclex_0 =	"9BDBE2",
  fnclex_0 =	"DBE2",

  fnop_0 =	"D9D0",
  -- D9D1-D9DF: unassigned

  fchs_0 =	"D9E0",
  fabs_0 =	"D9E1",
  -- D9E2: unassigned
  -- D9E3: unassigned
  ftst_0 =	"D9E4",
  fxam_0 =	"D9E5",
  -- D9E6: unassigned
  -- D9E7: unassigned
  fld1_0 =	"D9E8",
  fldl2t_0 =	"D9E9",
  fldl2e_0 =	"D9EA",
  fldpi_0 =	"D9EB",
  fldlg2_0 =	"D9EC",
  fldln2_0 =	"D9ED",
  fldz_0 =	"D9EE",
  -- D9EF: unassigned

  f2xm1_0 =	"D9F0",
  fyl2x_0 =	"D9F1",
  fptan_0 =	"D9F2",
  fpatan_0 =	"D9F3",
  fxtract_0 =	"D9F4",
  fprem1_0 =	"D9F5",
  fdecstp_0 =	"D9F6",
  fincstp_0 =	"D9F7",
  fprem_0 =	"D9F8",
  fyl2xp1_0 =	"D9F9",
  fsqrt_0 =	"D9FA",
  fsincos_0 =	"D9FB",
  frndint_0 =	"D9FC",
  fscale_0 =	"D9FD",
  fsin_0 =	"D9FE",
  fcos_0 =	"D9FF",
}

------------------------------------------------------------------------------

-- Arithmetic ops.
for name,n in pairs{ add = 0, ["or"] = 1, adc = 2, sbb = 3,
		     ["and"] = 4, sub = 5, xor = 6, cmp = 7 } do
  local n8 = n * 8
  map_op[name.."_2"] = format(
    "mr:%02XRm|rm:%02XrM|mI1dw:81%XmI|mS1dw:83%XmS|Ri1dwb:%02Xi|mi1dwb:81%Xmi",
    1+n8, 3+n8, n, n, 5+n8, n)
end

-- Shift ops.
for name,n in pairs{ rol = 0, ror = 1, rcl = 2, rcr = 3,
		     shl = 4, shr = 5,          sar = 7, sal = 4 } do
  map_op[name.."_2"] = format("m1:D1%Xm|mC1dwb:D3%Xm|mi:C1%XmU", n, n, n)
end

-- Conditional ops.
for cc,n in pairs(map_cc) do
  map_op["j"..cc.."_1"] = format("J.:0F8%XJ", n) -- short: 7%X
  map_op["set"..cc.."_1"] = format("mb:n0F9%X2m", n)
  map_op["cmov"..cc.."_2"] = format("rmdw:0F4%XrM", n) -- P6+
end

-- FP arithmetic ops.
for name,n in pairs{ add = 0, mul = 1, com = 2, comp = 3,
		     sub = 4, subr = 5, div = 6, divr = 7 } do
  local nc = 192 + n * 8
  local nr = nc + (n < 4 and 0 or (n % 2 == 0 and 8 or -8))
  local fn = "f"..name
  map_op[fn.."_1"] = format("ff:D8%02Xr|xdq:D8%Xm", nc, n)
  if n == 2 or n == 3 then
    map_op[fn.."_2"] = format("Fff:D8%02XR|Fx2dq:D8%XM", nc, n)
  else
    map_op[fn.."_2"] = format("Fff:D8%02XR|fFf:DC%02Xr|Fx2dq:D8%XM", nc, nr, n)
    map_op[fn.."p_1"] = format("ff:DE%02Xr", nr)
    map_op[fn.."p_2"] = format("fFf:DE%02Xr", nr)
  end
  map_op["fi"..name.."_1"] = format("xd:DA%Xm|xw:nDE%Xm", n, n)
end

-- FP conditional moves.
for cc,n in pairs{ b=0, e=1, be=2, u=3, nb=4, ne=5, nbe=6, nu=7 } do
  local n4 = n % 4
  local nc = 56000 + n4 * 8 + (n-n4) * 64
  map_op["fcmov"..cc.."_1"] = format("ff:%04Xr", nc) -- P6+
  map_op["fcmov"..cc.."_2"] = format("Fff:%04XR", nc) -- P6+
end

-- Hand code movzx, movsx. Templates do not support mixed operand sizes (yet).
function map_op.movzx_2(params)
  if not params then return "reg, mrm" end
  local opcode = params.op == "movzx" and 4023 or 4031
  local d = parseoperand(params[1])
  local s = parseoperand(params[2])
  local sz = (d.opsize or "")..(s.opsize or "")
  if match(d.mode, "r") and match(s.mode, "m") and
     (sz == "dw" or sz == "db" or sz == "wb") then
    wputop(d.opsize, s.opsize == "b" and opcode-1 or opcode)
    wputmrmsib(s, d.reg)
  else
    werror("bad operand mode in `"..opmodestr(params.op, {d, s}).."'")
  end
end
map_op.movsx_2 = map_op.movzx_2

------------------------------------------------------------------------------

-- Process pattern string.
local function dopattern(pat, args, sz, op)
  local digit, addin
  local opcode = 0
  local szov = sz

  -- Limit number of section buffer positions used by a single dasm_put().
  -- A single opcode needs a maximum of 2 positions. !x64
  if secpos+2 > maxsecpos then wflush() end

  -- Process each character.
  for c in gmatch(pat, ".") do
    if match(c, "%x") then	-- Hex digit.
      digit = byte(c) - 48
      if digit > 48 then digit = digit - 39
      elseif digit > 16 then digit = digit - 7 end
      opcode = opcode*16 + digit
      addin = nil
    elseif c == "n" then	-- Disable operand size mods for opcode.
      szov = nil
    elseif c == "r" then	-- Merge 1st operand regno. into opcode.
      addin = args[1].reg; opcode = opcode + addin
    elseif c == "R" then	-- Merge 2nd operand regno. into opcode.
      addin = args[2].reg; opcode = opcode + addin
    elseif c == "m" or c == "M" then	-- Encode ModRM/SIB.
      if addin then
	opcode = opcode - addin		-- Undo regno opcode merge.
      else
	addin = opcode % 16		-- Undo last digit.
	opcode = (opcode - addin) / 16
      end
      wputop(szov, opcode); opcode = nil
      local imark = (sub(pat, -1) == "I") -- Force a mark (ugly).
      -- Put ModRM/SIB with regno/last digit as spare.
      wputmrmsib(args[c == "m" and 1 or 2], addin, imark)
    else
      if opcode then wputop(szov, opcode); opcode = nil end -- Flush opcode.
      if c == "o" or c == "O" then	-- Offset (pure 32 bit displacement).
	wputdarg(args[c == "o" and 1 or 2].disp)
      else
	-- Anything else is an immediate operand.
	local a = args[#args]
	local mode, imm = a.mode, a.imm
	if mode == "iJ" and not match("iIJ", c) then
	  werror("bad operand size for label")
	end
	if c == "S" then
	  wputsbarg(imm)
	elseif c == "U" then
	  wputbarg(imm)
	elseif c == "W" then
	  wputwarg(imm)
	elseif c == "i" or c == "I" then
	  if mode == "iJ" then
	    wputlabel("IMM_", imm, 1)
	  elseif mode == "iI" and c == "I" then
	    waction(sz == "w" and "IMM_WB" or "IMM_DB", imm)
	  else
	    wputszarg(sz, imm)
	  end
	elseif c == "J" then
	  if mode == "iPJ" then
	    waction("REL_G", imm) -- !x64 (secpos)
	  else
	    wputlabel("REL_", imm, 2)
	  end
	else
	  werror("bad char `"..c.."' in pattern `"..pat.."' for `"..op.."'")
	end
      end
    end
  end
  if opcode then wputop(szov, opcode) end
end

------------------------------------------------------------------------------

-- Mapping of operand modes to short names. Suppress output with '#'.
local map_modename = {
  r = "reg", R = "eax", C = "cl", x = "mem", m = "mrm", i = "imm",
  f = "stx", F = "st0", J = "lbl", ["1"] = "1",
  I = "#", S = "#", O = "#",
}

-- Return a table/string showing all possible operand modes.
local function templatehelp(template, nparams)
  if nparams == 0 then return "" end
  local t = {}
  for tm in gmatch(template, "[^%|]+") do
    local s = map_modename[sub(tm, 1, 1)]
    s = s..gsub(sub(tm, 2, nparams), ".", function(c)
      return ", "..map_modename[c]
    end)
    if not match(s, "#") then t[#t+1] = s end
  end
  return t
end

-- Match operand modes against mode match part of template.
local function matchtm(tm, args)
  for i=1,#args do
    if not match(args[i].mode, sub(tm, i, i)) then return end
  end
  return true
end

-- Handle opcodes defined with template strings.
map_op[".template__"] = function(params, template, nparams)
  if not params then return templatehelp(template, nparams) end
  local args = {}

  -- Zero-operand opcodes have no match part.
  if #params == 0 then
    dopattern(template, args, "d", params.op)
    return
  end

  -- Determine common operand size (coerce undefined size) or flag as mixed.
  local sz, szmix
  for i,p in ipairs(params) do
    args[i] = parseoperand(p)
    local nsz = args[i].opsize
    if nsz then
      if sz and sz ~= nsz then szmix = true else sz = nsz end
    end
  end

  -- Try all match:pattern pairs (separated by '|').
  local gotmatch
  for tm in gmatch(template, "[^%|]+") do
    if matchtm(tm, args) then
      -- Split off size match (starts after mode match) and pattern string.
      local szm, pat = match(tm, "^(.-):(.*)$", #args+1)
      if szm == "" then szm = "dwb" end -- Default size match.
      local szp = sz
      local prefix = sub(szm, 1, 1)
      if prefix == "1" then szp = args[1].opsize; szmix = nil
      elseif prefix == "2" then szp = args[2].opsize; szmix = nil end
      -- Match operand size.
      if not szmix and (prefix == "." or match(szm, szp or "#")) then
	-- Process pattern string.
	dopattern(pat, args, szp, params.op)
	return
      end
      gotmatch = true
    end
  end

  local msg = "bad operand mode"
  if gotmatch then
    if szmix then
      msg = "mixed operand size"
    else
      msg = sz and "bad operand size" or "missing operand size"
    end
  end

  werror(msg.." in `"..opmodestr(params.op, args).."'")
end

------------------------------------------------------------------------------

-- Pseudo-opcodes for data storage.
local function op_data(params)
  if not params then return "imm..." end
  local sz = sub(params.op, 2, 2)
  if sz == "a" then sz = addrsize end
  for _,p in ipairs(params) do
    local a = parseoperand(p)
    if sub(a.mode, 1, 1) ~= "i" or a.mode == "iJ" or
       (a.opsize and a.opsize ~= sz) then
      werror("bad mode or size in `"..p.."'")
    end
    wputszarg(sz, a.imm)
  end
end

map_op[".byte_*"] = op_data
map_op[".sbyte_*"] = op_data
map_op[".word_*"] = op_data
map_op[".dword_*"] = op_data
map_op[".aword_*"] = op_data

------------------------------------------------------------------------------

-- Label pseudo-opcode (converted from trailing colon form).
map_op[".label_1"] = function(params)
  if not params then return "[1-9] | ->pc" end
  local a = parseoperand(params[1])
  local mode, imm = a.mode, a.imm
  if type(imm) == "number" then
    if imm >= 1 and imm <= 9 then
      -- Local label (1: ... 9:).
      waction("LABEL_L", nil, 1)
      wputxb(imm)
      return
    end
  elseif mode == "iJ" then
    -- PC label (->expr:).
    waction("LABEL_P", imm)
    return
  end
  werror("bad label definition")
end

------------------------------------------------------------------------------

-- Alignment pseudo-opcode.
map_op[".align_1"] = function( params)
  if not params then return "numpow2" end
  local align = tonumber(params[1])
  if align then
    local x = align
    -- Must be a power of 2 in the range (2 ... 256).
    for i=1,8 do
      x = x / 2
      if x == 1 then
	waction("ALIGN", nil, 1)
	wputxb(align-1) -- Action byte is 2**n-1.
	return
      end
    end
  end
  werror("bad alignment")
end

------------------------------------------------------------------------------

-- Pseudo-opcode for (primitive) type definitions (map to C types).
map_op[".type_3"] = function(params, nparams)
  if not params then
    return nparams == 2 and "name, ctype" or "name, ctype, reg"
  end
  local name, ctype, reg = params[1], params[2], params[3]
  if not match(name, "^[%a_][%w_]*$") then
    werror("bad type name `"..name.."'")
  end
  local tp = map_type[name]
  if tp then
    werror("duplicate type `"..name.."'")
  end
  if reg and not map_reg_valid_base[reg] then
    werror("bad base register `"..(map_reg_rev[reg] or reg).."'")
  end
  -- Add #type to defines. A bit unclean to put it in map_archdef.
  map_archdef["#"..name] = "sizeof("..ctype..")"
  -- Add new type and emit shortcut define.
  local num = ctypenum + 1
  map_type[name] = {
    ctype = ctype,
    ctypefmt = format("Dt%X(%%s)", num),
    reg = reg,
  }
  wline(format("#define Dt%X(_V) (int)&(((%s *)0)_V)", num, ctype))
  ctypenum = num
end
map_op[".type_2"] = map_op[".type_3"]

-- Dump type definitions.
local function dumptypes(out, lvl)
  local t = {}
  for name in pairs(map_type) do t[#t+1] = name end
  sort(t)
  out:write("Type definitions:\n")
  for _,name in ipairs(t) do
    local tp = map_type[name]
    local reg = tp.reg and map_reg_rev[tp.reg] or ""
    out:write(format("  %-20s %-20s %s\n", name, tp.ctype, reg))
  end
  out:write("\n")
end

------------------------------------------------------------------------------

-- Set the current section.
function _M.section(num)
  waction("SECTION")
  wputxb(num)
  wflush(true) -- SECTION is a terminal action.
end

------------------------------------------------------------------------------

-- Dump architecture description.
function _M.dumparch(out)
  out:write(format("DynASM %s version %s, released %s\n\n",
    _info.arch, _info.version, _info.release))
  dumpregs(out)
  dumpactions(out)
end

-- Dump all user defined elements.
function _M.dumpdef(out, lvl)
  dumptypes(out, lvl)
end

------------------------------------------------------------------------------

-- Pass callbacks from/to the DynASM core.
function _M.passcb(wl, we, wf, ww)
  wline, werror, wfatal, wwarn = wl, we, wf, ww
  return wflush
end

-- Setup the arch-specific module.
function _M.setup(arch, opt)
  g_arch, g_opt = arch, opt
end

-- Merge the core maps and the arch-specific maps.
function _M.mergemaps(map_coreop, map_def)
  setmetatable(map_op, { __index = map_coreop })
  setmetatable(map_def, { __index = map_archdef })
  return map_op, map_def
end

return _M

------------------------------------------------------------------------------

