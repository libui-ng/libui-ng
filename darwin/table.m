// 3 june 2018
#import "uipriv_darwin.h"
#import "table.h"

// TODO is the initial scroll position still wrong?

@interface uiprivTableModel : NSObject<NSTableViewDataSource, NSTableViewDelegate> {
	uiTableModel *m;
}
- (id)initWithModel:(uiTableModel *)model;
- (NSIndexSet *)tableView:(NSTableView *)tv selectionIndexesForProposedSelection:(NSIndexSet *)set;
@end

// TODO we really need to clean up the sharing of the table and model variables...
@interface uiprivTableView : NSTableView {
	uiTable *uiprivT;
	uiTableModel *uiprivM;
	NSTableHeaderView *headerViewRef;
}
@property uiTableSelectionMode selectionMode;

- (id)initWithFrame:(NSRect)r uiprivT:(uiTable *)t uiprivM:(uiTableModel *)m;
- (uiTable *)uiTable;
- (void)restoreHeaderView;
- (void)onClicked:(id)sender;
- (void)onDoubleClicked:(id)sender;
@end

@implementation uiprivTableView

@synthesize selectionMode;

- (id)initWithFrame:(NSRect)r uiprivT:(uiTable *)t uiprivM:(uiTableModel *)m
{
	self = [super initWithFrame:r];
	if (self) {
		self->uiprivT = t;
		self->uiprivM = m;
		self->headerViewRef = [self headerView];
	}
	return self;
}

- (uiTable *)uiTable
{
	return self->uiprivT;
}

- (void)restoreHeaderView
{
	[self setHeaderView:self->headerViewRef];
}

- (void)onClicked:(id)sender
{
	uiTable *t = self->uiprivT;
	NSInteger row = [self clickedRow];

	if (row < 0)
		return;

	(*(t->onRowClicked))(t, row, t->onRowClickedData);
}

- (void)onDoubleClicked:(id)sender
{
	uiTable *t = self->uiprivT;
	NSInteger row = [self clickedRow];

	if (row < 0)
		return;

	(*(t->onRowDoubleClicked))(t, row, t->onRowDoubleClickedData);
}

// TODO is this correct for overflow scrolling?
static void setBackgroundColor(uiprivTableView *t, NSTableRowView *rv, NSInteger row)
{
	NSColor *color;
	double r, g, b, a;

	if (t->uiprivT->backgroundColumn == -1)
		return;		// let Cocoa do its default thing
	if (uiprivTableModelColorIfProvided(t->uiprivM, row, t->uiprivT->backgroundColumn, &r, &g, &b, &a))
		color = [NSColor colorWithSRGBRed:r green:g blue:b alpha:a];
	else {
		NSArray *colors;
		NSInteger index;

		// this usage is primarily a guess; hopefully it is correct for the non-two color case... (TODO)
		// it does seem to be correct for the two-color case, judging from comparing against the value of backgroundColor before changing it (and no, nil does not work; it just sets to white)
		colors = [NSColor controlAlternatingRowBackgroundColors];
		index = row % [colors count];
		color = (NSColor *) [colors objectAtIndex:index];
	}
	[rv setBackgroundColor:color];
	// color is autoreleased in all cases
}

@end

@implementation uiprivTableModel

- (id)initWithModel:(uiTableModel *)model
{
	self = [super init];
	if (self)
		self->m = model;
	return self;
}

- (NSIndexSet *)tableView:(uiprivTableView *)tv selectionIndexesForProposedSelection:(NSIndexSet *)set
{
	if ([tv selectionMode] == uiTableSelectionModeNone)
		return nil;
	return set;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tv
{
	return uiprivTableModelNumRows(self->m);
}

 - (NSView *)tableView:(NSTableView *)tv viewForTableColumn:(NSTableColumn *)cc row:(NSInteger)row
{
	uiprivTableColumn *c = (uiprivTableColumn *) cc;
	uiprivTableCellView *cv;

	cv = (uiprivTableCellView *) [tv makeViewWithIdentifier:[c identifier] owner:self];
	if (cv == nil)
		cv = [c uiprivMakeCellView];
	[cv uiprivUpdate:row];
	return cv;
}

- (void)tableView:(NSTableView *)tv didAddRowView:(NSTableRowView *)rv forRow:(NSInteger)row
{
	setBackgroundColor((uiprivTableView *) tv, rv, row);
}

- (void)tableView:(uiprivTableView *)tv didClickTableColumn:(NSTableColumn *) tc
{
	uiTable *t = [tv uiTable];
	t->headerOnClicked(t, [[tc identifier] intValue], t->headerOnClickedData);
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
	uiTable *t = [(uiprivTableView*)[notification object] uiTable];
	t->onSelectionChanged(t, t->onSelectionChangedData);
}

@end

uiTableModel *uiNewTableModel(uiTableModelHandler *mh)
{
	uiTableModel *m;

	m = uiprivNew(uiTableModel);
	m->mh = mh;
	m->m = [[uiprivTableModel alloc] initWithModel:m];
	m->tables = [NSMutableArray new];
	return m;
}

void uiFreeTableModel(uiTableModel *m)
{
	if ([m->tables count] != 0)
		uiprivUserBug("You cannot free a uiTableModel while uiTables are using it.");
	[m->tables release];
	[m->m release];
	uiprivFree(m);
}

void uiTableModelRowInserted(uiTableModel *m, int newIndex)
{
	NSTableView *tv;
	NSIndexSet *set;

	set = [NSIndexSet indexSetWithIndex:newIndex];
	for (tv in m->tables)
		[tv insertRowsAtIndexes:set withAnimation:NSTableViewAnimationEffectNone];
	// set is autoreleased
}

void uiTableModelRowChanged(uiTableModel *m, int index)
{
	uiprivTableView *tv;
	NSTableRowView *rv;
	NSUInteger i, n;
	uiprivTableCellView *cv;

	for (tv in m->tables) {
		rv = [tv rowViewAtRow:index makeIfNecessary:NO];
		if (rv != nil)
			setBackgroundColor(tv, rv, index);
		n = [[tv tableColumns] count];
		for (i = 0; i < n; i++) {
			cv = (uiprivTableCellView *) [tv viewAtColumn:i row:index makeIfNecessary:NO];
			if (cv != nil)
				[cv uiprivUpdate:index];
		}
	}
}

void uiTableModelRowDeleted(uiTableModel *m, int oldIndex)
{
	NSTableView *tv;
	NSIndexSet *set;

	set = [NSIndexSet indexSetWithIndex:oldIndex];
	for (tv in m->tables)
		[tv removeRowsAtIndexes:set withAnimation:NSTableViewAnimationEffectNone];
	// set is autoreleased
}

uiTableModelHandler *uiprivTableModelHandler(uiTableModel *m)
{
	return m->mh;
}

uiDarwinControlAllDefaultsExceptDestroy(uiTable, sv)

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);

	[t->m->tables removeObject:t->tv];
	uiprivScrollViewFreeData(t->sv, t->d);
	[t->tv release];
	[t->sv release];
	uiFreeControl(uiControl(t));
}

int uiTableHeaderVisible(uiTable *t)
{
	return [t->tv headerView] != nil;
}

void uiTableHeaderSetVisible(uiTable *t, int visible)
{
	if (visible)
		[(uiprivTableView*)t->tv restoreHeaderView];
	else
		[t->tv setHeaderView:nil];
}

void uiTableHeaderOnClicked(uiTable *t, void (*f)(uiTable *, int, void *), void *data)
{
	t->headerOnClicked = f;
	t->headerOnClickedData = data;
}

static void defaultHeaderOnClicked(uiTable *table, int column, void *data)
{
	// do nothing
}

static void defaultOnSelectionChanged(uiTable *t, void *data)
{
	// do nothing
}

uiTableSelectionMode uiTableGetSelectionMode(uiTable *t)
{
	return [(uiprivTableView*)t->tv selectionMode];
}

void uiTableSetSelectionMode(uiTable *t, uiTableSelectionMode mode)
{
	switch (mode) {
		case uiTableSelectionModeNone:
			if ([t->tv numberOfSelectedRows] > 0)
				[t->tv deselectAll: t->tv];
			break;
		case uiTableSelectionModeZeroOrOne:
			if ([t->tv numberOfSelectedRows] > 1)
				[t->tv deselectAll: t->tv];
			[t->tv setAllowsMultipleSelection: NO];
			[t->tv setAllowsEmptySelection: YES];
			break;
		case uiTableSelectionModeOne:
			if ([t->tv numberOfSelectedRows] > 1)
				[t->tv deselectAll: t->tv];
			[t->tv setAllowsMultipleSelection: NO];
			[t->tv setAllowsEmptySelection: NO];
			break;
		case uiTableSelectionModeZeroOrMany:
			[t->tv setAllowsMultipleSelection: YES];
			[t->tv setAllowsEmptySelection: YES];
			break;
		default:
			uiprivUserBug("Invalid table selection mode %d", mode);
			return;
	}
	[(uiprivTableView*)t->tv setSelectionMode: mode];
}

void uiTableOnSelectionChanged(uiTable *t, void (*f)(uiTable *, void *), void *data)
{
	t->onSelectionChanged = f;
	t->onSelectionChangedData = data;
}

static void defaultOnRowClicked(uiTable *table, int row, void *data)
{
	// do nothing
}

static void defaultOnRowDoubleClicked(uiTable *table, int row, void *data)
{
	// do nothing
}

void uiTableOnRowClicked(uiTable *t, void (*f)(uiTable *, int, void *), void *data)
{
	t->onRowClicked = f;
	t->onRowClickedData = data;
}

void uiTableOnRowDoubleClicked(uiTable *t, void (*f)(uiTable *, int, void *), void *data)
{
	t->onRowDoubleClicked = f;
	t->onRowDoubleClickedData = data;
}

uiTableSelection* uiTableGetSelection(uiTable *t)
{
	__block int i = 0;
	NSIndexSet *set = [t->tv selectedRowIndexes];
	uiTableSelection *s = uiprivNew(uiTableSelection);

	s->NumRows = [set count];
	if (s->NumRows == 0)
		s->Rows = NULL;
	else
		s->Rows = uiprivAlloc(s->NumRows * sizeof(*s->Rows), "uiTableSelection->Rows");

	[set enumerateIndexesUsingBlock:^(NSUInteger row, BOOL *stop) {
		s->Rows[i++] = row;
	}];

	return s;
}

void uiTableSetSelection(uiTable *t, uiTableSelection *sel)
{
	int i;
	NSMutableIndexSet *set;
	uiTableSelectionMode mode = [(uiprivTableView*)t->tv selectionMode];

	if ((mode == uiTableSelectionModeNone && sel->NumRows > 0) ||
	    (mode == uiTableSelectionModeZeroOrOne && sel->NumRows > 1) ||
	    (mode == uiTableSelectionModeOne && sel->NumRows > 1)) {
		// TODO log error
		return;
	}

	set = [NSMutableIndexSet new];
	for (i = 0; i < sel->NumRows; ++i)
		[set addIndex: sel->Rows[i]];
	[t->tv selectRowIndexes: set byExtendingSelection: FALSE];
}

uiTable *uiNewTable(uiTableParams *p)
{
	uiTable *t;
	uiprivScrollViewCreateParams sp;

	uiDarwinNewControl(uiTable, t);
	t->m = p->Model;
	t->backgroundColumn = p->RowBackgroundColorModelColumn;

	t->tv = [[uiprivTableView alloc] initWithFrame:NSZeroRect uiprivT:t uiprivM:t->m];

	[t->tv setDataSource:t->m->m];
	[t->tv setDelegate:t->m->m];
	[t->tv reloadData];
	[t->m->tables addObject:t->tv];

	// TODO is this sufficient?
	[t->tv setAllowsColumnReordering:NO];
	[t->tv setAllowsColumnResizing:YES];
	[t->tv setAllowsColumnSelection:NO];
	[t->tv setUsesAlternatingRowBackgroundColors:YES];
	[t->tv setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleRegular];
	[t->tv setGridStyleMask:NSTableViewGridNone];
	[t->tv setAllowsTypeSelect:YES];
	// TODO floatsGroupRows — do we even allow group rows?

	uiTableOnRowClicked(t, defaultOnRowClicked, NULL);
	uiTableOnRowDoubleClicked(t, defaultOnRowDoubleClicked, NULL);
	[t->tv setAction: @selector(onClicked:)];
	[t->tv setDoubleAction: @selector(onDoubleClicked:)];

	memset(&sp, 0, sizeof (uiprivScrollViewCreateParams));
	sp.DocumentView = t->tv;
	// this is what Interface Builder sets it to
	// TODO verify
	sp.BackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
	sp.DrawsBackground = YES;
	sp.Bordered = YES;
	sp.HScroll = YES;
	sp.VScroll = YES;
	t->sv = uiprivMkScrollView(&sp, &(t->d));

	uiTableSetSelectionMode(t, uiTableSelectionModeZeroOrOne);
	uiTableHeaderOnClicked(t, defaultHeaderOnClicked, NULL);
	uiTableOnSelectionChanged(t, defaultOnSelectionChanged, NULL);

	// TODO WHY DOES THIS REMOVE ALL GRAPHICAL GLITCHES?
	// I got the idea from http://jwilling.com/blog/optimized-nstableview-scrolling/ but that was on an unrelated problem I didn't seem to have (although I have small-ish tables to start with)
	// I don't get layer-backing... am I supposed to layer-back EVERYTHING manually? I need to check Interface Builder again...
	[t->sv setWantsLayer:YES];

	return t;
}

uiSortIndicator uiTableHeaderSortIndicator(uiTable *t, int lcol)
{
	NSTableColumn *tc = [t->tv tableColumnWithIdentifier:[@(lcol) stringValue]];
	NSString *si = [[t->tv indicatorImageInTableColumn:tc] name];
	if ([si isEqualToString:@"NSAscendingSortIndicator"])
		return uiSortIndicatorAscending;
	else if ([si isEqualToString:@"NSDescendingSortIndicator"])
		return uiSortIndicatorDescending;
	return uiSortIndicatorNone;
}

void uiTableHeaderSetSortIndicator(uiTable *t, int lcol, uiSortIndicator indicator)
{
	NSTableColumn *tc = [t->tv tableColumnWithIdentifier:[@(lcol) stringValue]];
	NSImage *img;
	if (indicator == uiSortIndicatorAscending)
		img = [NSImage imageNamed:@"NSAscendingSortIndicator"];
	else if (indicator == uiSortIndicatorDescending)
		img = [NSImage imageNamed:@"NSDescendingSortIndicator"];
	else
		img = nil;
	[t->tv setIndicatorImage:img inTableColumn:tc];
}

int uiTableColumnWidth(uiTable *t, int column)
{
	NSTableColumn *tc = [t->tv tableColumnWithIdentifier:[@(column) stringValue]];
	return [tc width];
}

void uiTableColumnSetWidth(uiTable *t, int column, int width)
{
	NSTableColumn *tc = [t->tv tableColumnWithIdentifier:[@(column) stringValue]];

	if (width == -1)
		//TODO: resize not only to header but also to max content width
		[tc sizeToFit];
	else
		[tc setWidth: width];
}
