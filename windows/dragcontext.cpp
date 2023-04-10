#include "uipriv_windows.hpp"
#include "dragcontext.hpp"

int uiprivDropEffectsToDragOperations(DWORD de)
{
	int ops = uiDragOperationNone;

	if (de & DROPEFFECT_COPY)
		ops |= uiDragOperationCopy;
	if (de & DROPEFFECT_LINK)
		ops |= uiDragOperationLink;
	if (de & DROPEFFECT_MOVE)
		ops |= uiDragOperationMove;

	return ops;
}

void uiDragContextPosition(uiDragContext *dc, int *x, int *y)
{
	POINT pt = { dc->pt.x, dc->pt.y };

	ScreenToClient(dc->hwnd, &pt);
	*x = pt.x;
	*y = pt.y;
}

int uiDragContextDragTypes(uiDragContext *dc)
{
	int types = 0;
	ULONG n;
	FORMATETC fmtetc;
	IEnumFORMATETC *pefmtetc = NULL;

	// TODO see why this never triggers onMove()
	if (dc->pDataObj == NULL) {
		uiprivUserBug("You called uiDragContextDragTypes() from an illegal context");
		return 0;
	}

	if (dc->pDataObj->EnumFormatEtc(DATADIR_GET, &pefmtetc) == S_OK && pefmtetc != NULL) {
		while (pefmtetc->Next(1, &fmtetc, &n) == S_OK) {
			switch (fmtetc.cfFormat) {
				case CF_HDROP:
					types |= uiDragTypeURIs;
					break;
				case CF_UNICODETEXT:
				case CF_TEXT:
					types |= uiDragTypeText;
					break;
			}
		}
		pefmtetc->Release();
	}

	return types;
}

int uiDragContextDragOperations(uiDragContext *dc)
{
	return uiprivDropEffectsToDragOperations(*(dc->pdwEffect));
}

uiDragData* uiDragContextDragData(uiDragContext *dc, uiDragType type)
{
	FORMATETC fmtetc;
	STGMEDIUM medium;
	IDataObject* pDataObj = dc->pDataObj;
	uiDragData *d = NULL;

	if (!(dc->dd->typeMask & type)) {
		printf("ERROR: requesting unsupported drag type %d\n", type);
		return NULL;
	}

	switch (type) {
	case uiDragTypeURIs:
		{
			fmtetc = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
			medium = {};
			if(pDataObj->GetData(&fmtetc, &medium) == S_OK) {
				HDROP hDrop = (HDROP)medium.hGlobal;

				d = uiprivNew(uiDragData);
				d->type = uiDragTypeURIs;
				d->data.URIs.numURIs = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
				if (d->data.URIs.numURIs == 0)
					d->data.URIs.URIs = NULL;
				else
					d->data.URIs.URIs = (char**)uiprivAlloc(d->data.URIs.numURIs * sizeof(*d->data.URIs.URIs), "uiDragData->data.URIs.URIs");

				for (int i=0; i < d->data.URIs.numURIs; ++i) {
					WCHAR szName[MAX_PATH];

					DragQueryFile(hDrop, i, szName, MAX_PATH);
					d->data.URIs.URIs[i] = toUTF8(szName);
				}
			}
		}
		break;
	case uiDragTypeText:
		{
			fmtetc = {CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
			medium = {};
			if(pDataObj->GetData(&fmtetc, &medium) == S_OK) {
				void *data;

				d = uiprivNew(uiDragData);
				d->type = uiDragTypeText;
				data = GlobalLock(medium.hGlobal);
				d->data.text = toUTF8((WCHAR*)data);
				GlobalUnlock(data);
				break;
			}

			fmtetc = {CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
			medium = {};
			if(pDataObj->GetData(&fmtetc, &medium) == S_OK) {
				void *data;

				d = uiprivNew(uiDragData);
				d->type = uiDragTypeText;
				data = GlobalLock(medium.hGlobal);
				d->data.text = strdup((char*)data);
				GlobalUnlock(data);
			}
		}
		break;
	}

	return d;
}

