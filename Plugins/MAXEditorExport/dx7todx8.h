#ifndef _DX8TODX7_H_
#define _DX8TODX7_H_

///////////////////////////////////////////////////////////////////////////////
//
// d3d8types.h
//
///////////////////////////////////////////////////////////////////////////////

#define  D3DTS_WORLD  					D3DTRANSFORMSTATE_WORLD                 
#define  D3DTS_VIEW  					D3DTRANSFORMSTATE_VIEW                  
#define  D3DTS_PROJECTION  				D3DTRANSFORMSTATE_PROJECTION            
#define  D3DTS_WORLD1  					D3DTRANSFORMSTATE_WORLD1                
#define  D3DTS_WORLD2  					D3DTRANSFORMSTATE_WORLD2                
#define  D3DTS_WORLD3  					D3DTRANSFORMSTATE_WORLD3                
#define  D3DTS_TEXTURE0  				D3DTRANSFORMSTATE_TEXTURE0              
#define  D3DTS_TEXTURE1  				D3DTRANSFORMSTATE_TEXTURE1              
#define  D3DTS_TEXTURE2  				D3DTRANSFORMSTATE_TEXTURE2              
#define  D3DTS_TEXTURE3  				D3DTRANSFORMSTATE_TEXTURE3              
#define  D3DTS_TEXTURE4  				D3DTRANSFORMSTATE_TEXTURE4              
#define  D3DTS_TEXTURE5  				D3DTRANSFORMSTATE_TEXTURE5              
#define  D3DTS_TEXTURE6  				D3DTRANSFORMSTATE_TEXTURE6              
#define  D3DTS_TEXTURE7  				D3DTRANSFORMSTATE_TEXTURE7              
#define  D3DTS_FORCE_DWORD  			D3DTRANSFORMSTATE_FORCE_DWORD           

#define  D3DRS_ZENABLE  				D3DRENDERSTATE_ZENABLE                  
#define  D3DRS_FILLMODE  				D3DRENDERSTATE_FILLMODE                 
#define  D3DRS_SHADEMODE 				D3DRENDERSTATE_SHADEMODE                
#define  D3DRS_LINEPATTERN  			D3DRENDERSTATE_LINEPATTERN              
#define  D3DRS_ZWRITEENABLE 			D3DRENDERSTATE_ZWRITEENABLE             
#define  D3DRS_ALPHATESTENABLE  		D3DRENDERSTATE_ALPHATESTENABLE          
#define  D3DRS_LASTPIXEL  				D3DRENDERSTATE_LASTPIXEL                
#define  D3DRS_SRCBLEND  				D3DRENDERSTATE_SRCBLEND                 
#define  D3DRS_DESTBLEND  				D3DRENDERSTATE_DESTBLEND                
#define  D3DRS_CULLMODE  				D3DRENDERSTATE_CULLMODE                 
#define  D3DRS_ZFUNC  					D3DRENDERSTATE_ZFUNC                    
#define  D3DRS_ALPHAREF  				D3DRENDERSTATE_ALPHAREF                 
#define  D3DRS_ALPHAFUNC  				D3DRENDERSTATE_ALPHAFUNC                
#define  D3DRS_DITHERENABLE  			D3DRENDERSTATE_DITHERENABLE             
#define  D3DRS_ALPHABLENDENABLE  		D3DRENDERSTATE_ALPHABLENDENABLE         
#define  D3DRS_FOGENABLE  				D3DRENDERSTATE_FOGENABLE                
#define  D3DRS_SPECULARENABLE  			D3DRENDERSTATE_SPECULARENABLE           
#define  D3DRS_ZVISIBLE  				D3DRENDERSTATE_ZVISIBLE                 
#define  D3DRS_FOGCOLOR  				D3DRENDERSTATE_FOGCOLOR                 
#define  D3DRS_FOGTABLEMODE  			D3DRENDERSTATE_FOGTABLEMODE             
#define  D3DRS_FOGSTART  				D3DRENDERSTATE_FOGSTART                 
#define  D3DRS_FOGEND  					D3DRENDERSTATE_FOGEND                   
#define  D3DRS_FOGDENSITY  				D3DRENDERSTATE_FOGDENSITY               
#define  D3DRS_EDGEANTIALIAS  			D3DRENDERSTATE_EDGEANTIALIAS            
#define  D3DRS_ZBIAS  					D3DRENDERSTATE_ZBIAS                    
#define  D3DRS_RANGEFOGENABLE  			D3DRENDERSTATE_RANGEFOGENABLE           
#define  D3DRS_STENCILENABLE  			D3DRENDERSTATE_STENCILENABLE            
#define  D3DRS_STENCILFAIL  			D3DRENDERSTATE_STENCILFAIL              
#define  D3DRS_STENCILZFAIL  			D3DRENDERSTATE_STENCILZFAIL             
#define  D3DRS_STENCILPASS  			D3DRENDERSTATE_STENCILPASS              
#define  D3DRS_STENCILFUNC  			D3DRENDERSTATE_STENCILFUNC              
#define  D3DRS_STENCILREF  				D3DRENDERSTATE_STENCILREF               
#define  D3DRS_STENCILMASK  			D3DRENDERSTATE_STENCILMASK              
#define  D3DRS_STENCILWRITEMASK  		D3DRENDERSTATE_STENCILWRITEMASK         
#define  D3DRS_TEXTUREFACTOR  			D3DRENDERSTATE_TEXTUREFACTOR            
#define  D3DRS_WRAP0  					D3DRENDERSTATE_WRAP0                    
#define  D3DRS_WRAP1  					D3DRENDERSTATE_WRAP1                    
#define  D3DRS_WRAP2  					D3DRENDERSTATE_WRAP2                    
#define  D3DRS_WRAP3  					D3DRENDERSTATE_WRAP3                    
#define  D3DRS_WRAP4  					D3DRENDERSTATE_WRAP4                    
#define  D3DRS_WRAP5  					D3DRENDERSTATE_WRAP5                    
#define  D3DRS_WRAP6  					D3DRENDERSTATE_WRAP6                    
#define  D3DRS_WRAP7  					D3DRENDERSTATE_WRAP7                    
#define  D3DRS_CLIPPING  				D3DRENDERSTATE_CLIPPING                 
#define  D3DRS_LIGHTING  				D3DRENDERSTATE_LIGHTING                 
#define  D3DRS_EXTENTS  				D3DRENDERSTATE_EXTENTS                  
#define  D3DRS_AMBIENT  				D3DRENDERSTATE_AMBIENT                  
#define  D3DRS_FOGVERTEXMODE  			D3DRENDERSTATE_FOGVERTEXMODE            
#define  D3DRS_COLORVERTEX  			D3DRENDERSTATE_COLORVERTEX              
#define  D3DRS_LOCALVIEWER  			D3DRENDERSTATE_LOCALVIEWER              
#define  D3DRS_NORMALIZENORMALS  		D3DRENDERSTATE_NORMALIZENORMALS         
#define  D3DRS_DIFFUSEMATERIALSOURCE  	D3DRENDERSTATE_DIFFUSEMATERIALSOURCE    
#define  D3DRS_SPECULARMATERIALSOURCE  	D3DRENDERSTATE_SPECULARMATERIALSOURCE   
#define  D3DRS_AMBIENTMATERIALSOURCE  	D3DRENDERSTATE_AMBIENTMATERIALSOURCE    
#define  D3DRS_EMISSIVEMATERIALSOURCE  	D3DRENDERSTATE_EMISSIVEMATERIALSOURCE   
#define  D3DRS_VERTEXBLEND  			D3DRENDERSTATE_VERTEXBLEND              
#define  D3DRS_CLIPPLANEENABLE  		D3DRENDERSTATE_CLIPPLANEENABLE          

#define  D3DCOLOR_RGBA  				RGBA_MAKE                               
#define  D3DCOLOR_XRGB  				RGB_MAKE                                
#define  D3DCOLOR_COLORVALUE  			D3DRGBA                                 

#endif //_DX7TODX8_H_
