#pragma once

// Singleton template definition 
template <class T> class CSingleton {
	static T*	_self;
	static int	_refcount;
public:
					CSingleton	()	{}
	virtual			~CSingleton	()	{_self=NULL;}
public:
	static	T*		Instance	() {
		if(!_self) _self=xr_new<T>(); 
		++_refcount;
		return _self;
	}
	void			FreeInst	() {
		if(0 == --_refcount) {
			CSingleton<T> *ptr = this;
			xr_delete(ptr);
		} 
	}
};

template <class T> T*	CSingleton<T>::_self		= NULL;
template <class T> int	CSingleton<T>::_refcount	= 0;


template<class SHARED_TYPE, class KEY_TYPE> class CSharedObj : public CSingleton<CSharedObj<SHARED_TYPE, KEY_TYPE> >
{
	xr_map<KEY_TYPE, SHARED_TYPE*> _shared_tab;	
	typedef typename xr_map<KEY_TYPE, SHARED_TYPE*>::iterator SHARED_DATA_MAP_IT;

public:
				CSharedObj	() {};
	virtual		~CSharedObj	() {
		for (SHARED_DATA_MAP_IT it = _shared_tab.begin(); it != _shared_tab.end(); ++it){
			xr_delete(it->second);
		}
	}

	// Access to data
	SHARED_TYPE	*get_shared	(KEY_TYPE id) {
		SHARED_DATA_MAP_IT shared_it = _shared_tab.find(id);

		SHARED_TYPE *_data;

		// if not found - create appropriate shared data object
		if (_shared_tab.end() == shared_it) {
			_data		= xr_new<SHARED_TYPE>();
			_shared_tab.insert(mk_pair(id, _data));
		} else _data = shared_it->second;

		return _data;
	}
};

class CSharedResource {
	bool	loaded;
public:
			CSharedResource	() {loaded = false;}

	bool	IsLoaded		() {return loaded;}
	void	SetLoad			(bool l = true) {loaded = l;}
};


template<class SHARED_TYPE, class KEY_TYPE> class CSharedClass {
	SHARED_TYPE							*_sd;
	CSharedObj<SHARED_TYPE, KEY_TYPE>	*pSharedObj;
public:
					CSharedClass	() {pSharedObj	= CSharedObj<SHARED_TYPE,KEY_TYPE>::Instance();}
	virtual			~CSharedClass	() {pSharedObj->FreeInst();}

	void			load_shared		(KEY_TYPE key, LPCSTR section) {
		_sd = pSharedObj->get_shared(key);

		if (!get_sd()->IsLoaded()) {
			load_shared(section);
			get_sd()->SetLoad();
		}
	}

	virtual void	load_shared	(LPCSTR section) {}

	SHARED_TYPE		*get_sd			() {return _sd;}
	
	
	// ���������� ��������� ������ ��� ������������ ������� (�������� ������ �������)
	bool start_load_shared	 (KEY_TYPE key){ 
		_sd = pSharedObj->get_shared(key);
		if (get_sd()->IsLoaded()) return false;
		return true;
	}
	void finish_load_shared	 (){get_sd()->SetLoad();}

};


//-----------------------------------------
// Usage
//-----------------------------------------
////1. define shared class storage
//struct shared_struc : public CSharedResource {
//	u8 a;
//	u8 b;
//};
//
////2. define custom class inherited CSharedClass
//class CClass : public CSharedClass<shared_struc, CLASS_ID> {
//	typedef  CSharedClass<shared_struc, CLASS_ID> inherited_shared;
//	
//public:
//
//	//3. call shared load on Load
//	virtual void Load			(LPCSTR section) {
//		inherited_shared::load_shared(class_id(), section);
//	}
//	
//	//4. load shared data
//	virtual void load_shared	(LPCSTR section) {
//		a = pSettings->r_u8(section, "smth1");
//		b = pSettings->r_u8(section, "smth2");
//	}
//
//	//5. access to shared data
//	void Smth() {
//		Msg("shared: a = [%d] b =[%d]", inherited_shared::get_sd()->a, inherited_shared::get_sd()->b);
//	}
//};

