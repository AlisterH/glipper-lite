//extern variables:
extern int maxElements; 
extern int maxItemLength;
extern gboolean usePrimary; 
extern gboolean useDefault; 
extern gboolean markDefault; 
extern gboolean weSaveHistory;
extern char* keyComb;

extern GSList* history;

//extern functions:
void savePreferences();
void applyPreferences();
void unbindKey();
