#ifndef __LIBUI_DRAGCONTEXT_HPP__
#define __LIBUI_DRAGCONTEXT_HPP__

struct uiDragContext {
	uiDragDestination *dd;
	HWND hwnd;
	POINTL pt;
	DWORD *pdwEffect;
	IDataObject *pDataObj;
};

#endif

