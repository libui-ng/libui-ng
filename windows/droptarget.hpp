#ifndef __LIBUI_DROPTARGET_HPP
#define __LIBUI_DROPTARGET_HPP

#include <ole2.h>

class uiDropTarget : public IDropTarget
{
	public:
		uiDropTarget(uiDragDestination *dragDest, HWND hwnd);
		virtual ~uiDropTarget();

		//IUnknown
		STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();

		//IDropTarget
		STDMETHODIMP DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
		STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
		STDMETHODIMP DragLeave();
		STDMETHODIMP Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

	private:
		bool AcceptData(IDataObject* pDataObj);
		//Data
		uiDragDestination *m_DragDest;
		HWND m_HWND;
		IDataObject *m_DataObj;

		//IUnknown
		ULONG m_Ref;
};

#endif
