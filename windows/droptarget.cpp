#include "uipriv_windows.hpp"
#include "dragcontext.hpp"
#include "droptarget.hpp"
#include <shlobj.h>

static DWORD dragOperationToDropEffect(uiDragOperation op)
{
	switch (op) {
		case uiDragOperationNone:
			return DROPEFFECT_NONE;
		case uiDragOperationCopy:
			return DROPEFFECT_COPY;
		case uiDragOperationLink:
			return DROPEFFECT_LINK;
		case uiDragOperationMove:
			return DROPEFFECT_MOVE;
	}
	return DROPEFFECT_NONE;
}

uiDropTarget::uiDropTarget(uiDragDestination *dragDest, HWND hwnd)
{
	m_DragDest = dragDest;
	m_HWND = hwnd;
	m_Ref = 1;
}

uiDropTarget::~uiDropTarget()
{
}

STDMETHODIMP uiDropTarget::QueryInterface(REFIID riid, LPVOID *ppv)
{
	if(ppv == NULL)
		return E_POINTER;

	if (riid == IID_IUnknown || riid == IID_IDropTarget) {
		*ppv = this;
		AddRef();
		return S_OK;
	}

	*ppv = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) uiDropTarget::AddRef()
{
	return ++m_Ref;
}

STDMETHODIMP_(ULONG) uiDropTarget::Release()
{
	if (--m_Ref == 0) {
		delete this;
	}
	return m_Ref;
}

bool uiDropTarget::AcceptData(IDataObject* pDataObj)
{
	FORMATETC fmtetc;
	bool accept = false;

	if (m_DragDest->typeMask & uiDragTypeText) {
		fmtetc = {CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		if (pDataObj->QueryGetData(&fmtetc) == S_OK)
			accept = true;
		fmtetc = {CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		if (pDataObj->QueryGetData(&fmtetc) == S_OK)
			accept = true;
	}
	if (m_DragDest->typeMask & uiDragTypeURIs) {
		fmtetc = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		if (pDataObj->QueryGetData(&fmtetc) == S_OK)
			accept = true;
	}

	return accept;
}

STDMETHODIMP uiDropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	struct uiDragContext context = { m_DragDest, m_HWND, pt, pdwEffect, pDataObj };

	if (!AcceptData(pDataObj)) {
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	// Cache data object for use in ::DragOver which is legal according to:
	// https://learn.microsoft.com/en-us/windows/win32/api/oleidl/nf-oleidl-idroptarget-dragover
	m_DataObj = pDataObj;

	m_DragDest->op = uiDragOperationNone;
	m_DragDest->op = m_DragDest->onEnter(m_DragDest, &context, m_DragDest->onEnterData);

	*pdwEffect = dragOperationToDropEffect(m_DragDest->op);
	return S_OK;
}

STDMETHODIMP uiDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	struct uiDragContext context = { m_DragDest, m_HWND, pt, pdwEffect, m_DataObj };

	m_DragDest->op = m_DragDest->onMove(m_DragDest, &context, m_DragDest->onMoveData);

	*pdwEffect = dragOperationToDropEffect(m_DragDest->op);
	return S_OK;
}

STDMETHODIMP uiDropTarget::DragLeave()
{
	m_DragDest->onExit(m_DragDest, m_DragDest->onExitData);
	return S_OK;
}

STDMETHODIMP uiDropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	int success;
	struct uiDragContext context = { m_DragDest, m_HWND, pt, pdwEffect, pDataObj };

	success = m_DragDest->onDrop(m_DragDest, &context, m_DragDest->onDropData);
	if (!success)
		m_DragDest->op = uiDragOperationNone;

	*pdwEffect = dragOperationToDropEffect(m_DragDest->op);
	return S_OK;
}

