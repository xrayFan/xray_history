object frmImageLib: TfrmImageLib
  Left = 418
  Top = 274
  Width = 350
  Height = 455
  BorderIcons = [biSystemMenu, biMinimize]
  Caption = 'Image Editor'
  Color = 10528425
  Constraints.MinHeight = 400
  Constraints.MinWidth = 350
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  KeyPreview = True
  OldCreateOrder = False
  Scaled = False
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Splitter1: TSplitter
    Left = 168
    Top = 0
    Width = 2
    Height = 428
    Cursor = crHSplit
    Color = 3026478
    ParentColor = False
  end
  object paRight: TPanel
    Left = 170
    Top = 0
    Width = 172
    Height = 428
    Align = alClient
    BevelOuter = bvLowered
    Color = 10528425
    Constraints.MinWidth = 172
    TabOrder = 0
    object Bevel2: TBevel
      Left = 1
      Top = 351
      Width = 170
      Height = 2
      Align = alBottom
      Shape = bsBottomLine
    end
    object paCommand: TPanel
      Left = 1
      Top = 353
      Width = 170
      Height = 74
      Align = alBottom
      BevelInner = bvLowered
      BevelOuter = bvNone
      Color = 10528425
      TabOrder = 0
      object ebOk: TExtBtn
        Left = 1
        Top = 39
        Width = 168
        Height = 17
        Align = alTop
        BevelShow = False
        CloseButton = False
        Caption = 'Ok'
        FlatAlwaysEdge = True
        OnClick = ebOkClick
      end
      object Bevel1: TBevel
        Left = 1
        Top = 18
        Width = 168
        Height = 2
        Align = alTop
        Shape = bsLeftLine
        Style = bsRaised
      end
      object ebCancel: TExtBtn
        Left = 1
        Top = 56
        Width = 168
        Height = 17
        Align = alTop
        BevelShow = False
        CloseButton = False
        Caption = 'Cancel'
        FlatAlwaysEdge = True
        OnClick = ebCancelClick
      end
      object ebRemoveTexture: TExtBtn
        Left = 1
        Top = 1
        Width = 168
        Height = 17
        Align = alTop
        BevelShow = False
        CloseButton = False
        Caption = 'Remove Texture'
        FlatAlwaysEdge = True
        OnClick = ebRemoveTextureClick
      end
      object ebRebuildAssociation: TExtBtn
        Left = 1
        Top = 20
        Width = 168
        Height = 17
        Align = alTop
        BevelShow = False
        CloseButton = False
        Caption = 'Rebuild Association'
        FlatAlwaysEdge = True
        OnClick = ebRebuildAssociationClick
      end
      object Bevel5: TBevel
        Left = 1
        Top = 37
        Width = 168
        Height = 2
        Align = alTop
        Shape = bsLeftLine
        Style = bsRaised
      end
    end
    object paProperties: TPanel
      Left = 1
      Top = 133
      Width = 170
      Height = 218
      Align = alClient
      BevelOuter = bvNone
      Color = 10528425
      TabOrder = 1
    end
    object Panel1: TPanel
      Left = 1
      Top = 1
      Width = 170
      Height = 132
      Align = alTop
      BevelOuter = bvNone
      ParentColor = True
      TabOrder = 2
      object paImage: TMxPanel
        Left = 1
        Top = 1
        Width = 130
        Height = 130
        BevelOuter = bvLowered
        Caption = '<no image>'
        ParentColor = True
        TabOrder = 0
        OnPaint = paImagePaint
      end
    end
  end
  object paItems: TPanel
    Left = 0
    Top = 0
    Width = 168
    Height = 428
    Align = alLeft
    BevelOuter = bvNone
    Constraints.MinWidth = 168
    ParentColor = True
    TabOrder = 1
  end
  object fsStorage: TFormStorage
    Version = 1
    OnSavePlacement = fsStorageSavePlacement
    OnRestorePlacement = fsStorageRestorePlacement
    StoredProps.Strings = (
      'paRight.Width')
    StoredValues = <>
  end
  object ImageList: TImageList
    Height = 10
    Width = 11
    Left = 32
    Bitmap = {
      494C01010200040004000B000A00FFFFFFFFFF10FFFFFFFFFFFFFFFF424D3600
      00000000000036000000280000002C0000000A0000000100200000000000E006
      0000000000000000000000000000000000000000000000000000000000000000
      00000C0C57000C0C570000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000C0C57000C0C5700000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000C0C57000C0C570000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000002A2A57000C0C57000C0C57002A2A57000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000002A2A
      57000C0C57000C0C57002A2A5700000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000002A2A57000C0C57000C0C57002A2A57000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000002A2A
      57000C0C57000C0C57002A2A5700000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000C0C57000C0C5700000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000424D3E000000000000003E000000
      280000002C0000000A0000000100010000000000500000000000000000000000
      000000000000000000000000FFFFFF00F3FFFC0000000000F3FDFC0000000000
      FFF8FC0000000000FFF07C0000000000F3E33C0000000000E1E79C0000000000
      E1FFCC0000000000E1FFE40000000000E1FFF40000000000F3FFFC0000000000
      00000000000000000000000000000000000000000000}
  end
end
