#pragma once

#include <Windows.h>

typedef class SmartHandle {
	bool _init;
	HANDLE _handle;
public:	
	SmartHandle(){ 
		printf("Constructor called!\n");
		_init = false;
		_handle = NULL;
	};

	~SmartHandle();

	HANDLE get_Handle();

	void set_Handle(HANDLE handle){
		if(_init){
			throw "Already inited!";
		}
		if(handle == NULL){
			throw "Error getting handle!";
		}

		_init = true;
		_handle = handle;
	}

} SmartHandle, *PSmartHandle;


HANDLE SmartHandle::get_Handle(){
	return _handle;
}

SmartHandle::~SmartHandle(){	
	if(!CloseHandle(_handle)){
		throw "Error closing handle %d";
	}
	printf("Destructor called!\n");
}