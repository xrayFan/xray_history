// FS.h: interface for the CFS class.
//
//////////////////////////////////////////////////////////////////////

#ifndef fsH
#define fsH

#define CFS_CompressMark	(1ul << 31ul)

XRCORE_API void VerifyPath	(LPCSTR path);
//------------------------------------------------------------------------------------
// Write
//------------------------------------------------------------------------------------
class XRCORE_API IWriter
{
private:
	xr_stack<u32>	chunk_pos;
public:
	shared_str			fName;
public:
	IWriter	()
	{
	}
	virtual	~IWriter	()
	{
        R_ASSERT3	(chunk_pos.empty(),"Opened chunk not closed.",*fName);
	}

	// kernel
	virtual void	seek	(u32 pos)						= 0;
	virtual u32		tell	()								= 0;

	virtual void	w		(const void* ptr, u32 count)	= 0;

	// generalized writing functions
	IC void			w_u64	(u64 d)					{	w(&d,sizeof(u64));	}
	IC void			w_u32	(u32 d)					{	w(&d,sizeof(u32));	}
	IC void			w_u16	(u16 d)					{	w(&d,sizeof(u16));	}
	IC void			w_u8	(u8 d)					{	w(&d,sizeof(u8));	}
	IC void			w_s64	(s64 d)					{	w(&d,sizeof(s64));	}
	IC void			w_s32	(s32 d)					{	w(&d,sizeof(s32));	}
	IC void			w_s16	(s16 d)					{	w(&d,sizeof(s16));	}
	IC void			w_s8	(s8 d)					{	w(&d,sizeof(s8));	}
	IC void			w_float	(float d)				{	w(&d,sizeof(float));}
	IC void			w_string(const char *p)			{	w(p,(u32)xr_strlen(p));w_u8(13);w_u8(10);	}
	IC void			w_stringZ(const char *p)		{	w(p,(u32)xr_strlen(p)+1);					}
	IC void			w_stringZ(const shared_str& p) 	{	w(*p?*p:"",p.size());w_u8(0);		}
	IC void			w_stringZ(shared_str& p)		{	w(*p?*p:"",p.size());w_u8(0);		}
	IC void			w_stringZ(const std::string& p)	{	w(p.c_str()?p.c_str():"",(u32)p.size());w_u8(0);	}
	IC void			w_fcolor(const Fcolor &v)		{	w(&v,sizeof(Fcolor));	}
	IC void			w_fvector4(const Fvector4 &v)	{	w(&v,sizeof(Fvector4));	}
	IC void			w_fvector3(const Fvector3 &v)	{	w(&v,sizeof(Fvector3));	}
	IC void			w_fvector2(const Fvector2 &v)	{	w(&v,sizeof(Fvector2));	}
	IC void			w_ivector4(const Ivector4 &v)	{	w(&v,sizeof(Ivector4));	}
	IC void			w_ivector3(const Ivector3 &v)	{	w(&v,sizeof(Ivector3));	}
	IC void			w_ivector2(const Ivector2 &v)	{	w(&v,sizeof(Ivector2));	}

    // quant writing functions
	IC void 		w_float_q16	(float a, float min, float max)
	{
		VERIFY		(a>=min && a<=max);
		float q		= (a-min)/(max-min);
		w_u16		(u16(iFloor(q*65535.f+.5f)));
	}
	IC void 		w_float_q8	(float a, float min, float max)
	{
		VERIFY		(a>=min && a<=max);
		float q		= (a-min)/(max-min);
		w_u8		(u8(iFloor(q*255.f+.5f)));
	}
	IC void 		w_angle16	(float a)		    {	w_float_q16	(angle_normalize(a),0,PI_MUL_2);}
	IC void 		w_angle8	(float a)		    {	w_float_q8	(angle_normalize(a),0,PI_MUL_2);}
	IC void 		w_dir		(const Fvector& D) 	{	w_u16(pvCompress(D));	}
	void 			w_sdir		(const Fvector& D);
	void	__cdecl	w_printf	(const char* format, ...);

	// generalized chunking
	u32				align		();
	void			open_chunk	(u32 type);
	void			close_chunk	();
	u32				chunk_size	();					// returns size of currently opened chunk, 0 otherwise
	void			w_compressed(void* ptr, u32 count);
	void			w_chunk		(u32 type, void* data, u32 size);
};

class XRCORE_API CMemoryWriter : public IWriter
{
	u8*				data;
	u32				position;
	u32				mem_size;
	u32				file_size;
public:
	CMemoryWriter() {
		data		= 0;
		position	= 0;
		mem_size	= 0;
		file_size	= 0;
	}
	virtual	~CMemoryWriter();

	// kernel
	virtual void	w			(const void* ptr, u32 count);

	virtual void	seek		(u32 pos)	{	position = pos;				}
	virtual u32		tell		() 			{	return position;			}

	// specific
	IC u8*			pointer		()			{	return data;				}
	IC u32			size		() const 	{	return file_size;			}
	IC void			clear		()			{	file_size=0; position=0;	}
	void			save_to		(const char* fn);
};

//------------------------------------------------------------------------------------
// Read
//------------------------------------------------------------------------------------
class XRCORE_API IReader {
protected:
	char *			data;
	int				Pos;
	int				Size;

	IC u32			correction	(u32 p)
	{
		if (p%16) {
			return ((p%16)+1)*16 - p;
		} return 0;
	}
	u32 			advance_term_string	();
public:
	IReader			()
	{
		Pos			= 0;
	}
	IReader			(void *_data, int _size)
	{
		data		= (char *)_data;
		Size		= _size;
		Pos			= 0;
	};
	virtual ~IReader() {};

	IC int			elapsed		()	const		{	return Size-Pos;		};
	IC BOOL			eof			()	const		{	return elapsed()<=0;	};
	IC int			tell		()	const		{	return Pos;				};
	IC void			seek		(int ptr)		{	Pos=ptr; VERIFY((Pos<=Size) && (Pos>=0));};
	IC int			length		()	const		{	return Size;			};
	IC void*		pointer		()	const		{	return &(data[Pos]);	};
	IC void			advance		(int cnt)		{	Pos+=cnt;VERIFY((Pos<=Size) && (Pos>=0));};
	
	void			r			(void *p,int cnt);

	void			r_string	(char *dest, u32 tgt_sz);
	void			r_string	(std::string& dest);

	void			skip_stringZ();

	void			r_stringZ	(char *dest, u32 tgt_sz);
	void			r_stringZ	(shared_str& dest);
	void			r_stringZ	(std::string& dest);

	IC u64			r_u64		()			{	u64 tmp;	r(&tmp,sizeof(tmp)); return tmp;	};
	IC u32			r_u32		()			{	u32 tmp;	r(&tmp,sizeof(tmp)); return tmp;	};
	IC u16			r_u16		()			{	u16 tmp;	r(&tmp,sizeof(tmp)); return tmp;	};
	IC u8			r_u8		()			{	u8 tmp;		r(&tmp,sizeof(tmp)); return tmp;	};
	IC s64			r_s64		()			{	s64 tmp;	r(&tmp,sizeof(tmp)); return tmp;	};
	IC s32			r_s32		()			{	s32 tmp;	r(&tmp,sizeof(tmp)); return tmp;	};
	IC s16			r_s16		()			{	s16 tmp;	r(&tmp,sizeof(tmp)); return tmp;	};
	IC s8			r_s8		()			{	s8 tmp;		r(&tmp,sizeof(tmp)); return tmp;	};
	IC float		r_float		()			{	float tmp;	r(&tmp,sizeof(tmp)); return tmp;	};
	IC void			r_fvector4	(Fvector4 &v){	r(&v,sizeof(Fvector4));	}
	IC void			r_fvector3	(Fvector3 &v){	r(&v,sizeof(Fvector3));	}
	IC void			r_fvector2	(Fvector2 &v){	r(&v,sizeof(Fvector2));	}
	IC void			r_ivector4	(Ivector4 &v){	r(&v,sizeof(Ivector4));	}
	IC void			r_ivector4	(Ivector3 &v){	r(&v,sizeof(Ivector3));	}
	IC void			r_ivector4	(Ivector2 &v){	r(&v,sizeof(Ivector2));	}
	IC void			r_fcolor	(Fcolor &v)	{	r(&v,sizeof(Fcolor));	}
	
	IC float		r_float_q16	(float min, float max)
	{
		u16	val 	= r_u16();
		float A		= (float(val)*(max-min))/65535.f + min;		// floating-point-error possible
		VERIFY		((A >= min-EPS_S) && (A <= max+EPS_S));
        return A;
	}
	IC float		r_float_q8	(float min, float max)
	{
		u8 val		= r_u8();
		float	A	= (float(val)/255.0001f) *(max-min) + min;	// floating-point-error possible
		VERIFY		((A >= min) && (A <= max));
        return	A;
	}
	IC float		r_angle16	()			{ return r_float_q16(0,PI_MUL_2);	}
	IC float		r_angle8	()			{ return r_float_q8	(0,PI_MUL_2);	}
	IC void			r_dir		(Fvector& A){ u16 t=r_u16(); pvDecompress(A,t); }
	IC void			r_sdir		(Fvector& A)
	{
		u16	t		= r_u16();
		float s		= r_float();
		pvDecompress(A,t);
		A.mul		(s);
	}
	// Set file pointer to start of chunk data (0 for root chunk)
	IC void			rewind		()			{	seek(0); }
	
	// ����� XR Chunk'�� - ������� - ������ ��� 0
	u32 			find_chunk	(u32 ID, BOOL* bCompressed=0);	
	BOOL			r_chunk		(u32 ID, void *dest);					// ������ XR Chunk'�� (4b-ID,4b-size,??b-data)
	BOOL			r_chunk_safe(u32 ID, void *dest, u32 dest_size);	// ������ XR Chunk'�� (4b-ID,4b-size,??b-data)
	IReader*		open_chunk	(u32 ID);
	void			close		();
};

class XRCORE_API CVirtualFileRW : public IReader
{
private:
	void	*hSrcFile, *hSrcMap;
public:
			CVirtualFileRW		(const char *cFileName);
	virtual ~CVirtualFileRW		();
};

#endif // fsH
