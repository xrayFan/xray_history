#ifndef UI_MainCommandH
#define UI_MainCommandH

enum ECommands{
	COMMAND_INITIALIZE,
	COMMAND_DESTROY,
	COMMAND_EXIT,
	COMMAND_CLEAR,
	COMMAND_LOAD,
	COMMAND_SAVE,
	COMMAND_SAVEAS,
    COMMAND_IMPORT,

	COMMAND_UPDATE_GRID,
    COMMAND_GRID_NUMBER_OF_SLOTS,
    COMMAND_GRID_SLOT_SIZE,
    COMMAND_CHECK_MODIFIED,

	COMMAND_EDITOR_PREF,
	COMMAND_UPDATE_CAPTION,
	COMMAND_REFRESH_TEXTURES,	// p1 - refresh only new (true,false)
	COMMAND_RENDER_FOCUS,
	COMMAND_ZOOM_EXTENTS,
	COMMAND_UPDATE_TOOLBAR,
	COMMAND_RESET_ANIMATION,
	COMMAND_APPLY_CHANGES,

    COMMAND_CHANGE_ACTION,
    COMMAND_CHANGE_AXIS,
    // unused (only for compatibility)
    COMMAND_UNDO,
    COMMAND_REDO
    
};
#endif //UI_MainCommandH

