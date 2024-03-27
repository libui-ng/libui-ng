#import "uipriv_darwin.h"

NSDragOperation uiprivDragOperationToNSDragOperation(uiDragOperation op)
{
	switch (op) {
		case uiDragOperationNone:
			return NSDragOperationNone;
		case uiDragOperationCopy:
			return NSDragOperationCopy;
		case uiDragOperationLink:
			return NSDragOperationLink;
		case uiDragOperationMove:
			return NSDragOperationMove;
	}
	return NSDragOperationNone;
}

