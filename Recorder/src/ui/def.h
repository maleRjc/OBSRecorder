#pragma once
#ifndef RECORDER_SRC_UI_DEF_H_
#define RECORDER_SRC_UI_DEF_H_

/* File */
#define RESOURCE_PATH                                  (_T("resource"))
#define SKIN_FOLDER                                    (_T(""))
#define XML_RECORDER                                   (_T("recorder.xml"))
#define XML_TOOLBAR                                    (_T("toolbar.xml"))
#define XML_CAPTURE                                    (_T("capture.xml"))
#define XML_HEADER                                     (_T("header.xml"))
#define ZIP_RESOURCE                                   (_T("resource.zip"))

/* Windows */
#define WINDOW_CLASS_NAME                              (_T("Screen_Recorder_Within_Browser"))
#define WINDOW_NAME_RECORD                             (_T("Recorder"))
#define WINDOW_NAME_TOOLBAR                            (_T("Toolbar"))
#define WINDOW_NAME_CAPTURE                            (_T("Capture"))
#define WINDOW_NAME_HEADER                             (_T("Header"))

/* CControlUI */
#define DUI_CTR_RECORDER_BORDER                        (_T("RecorderBorder"))
#define DUI_CTR_RECORDER_CANVAS                        (_T("RecorderCanvas"))
#define DUI_CTR_CANVASCONTAINER                        (_T("CanvasContainer"))
#define DUI_CTR_HEADER_EDIT                            (_T("HeaderEdit"))
#define DUI_CTR_RECORDER_TEXT                          (_T("ReocrderText"))
#define DUI_CTR_TOOLBAR_TIME_BUTTON                    (_T("TimeButton"))

/* Flag */
#define DUI_CTR_COLORBTNTYPE                           (_T("colorbtntypeforvalidation"))

/* Common args */
#define COLOR_HIGHLIGHT_ARGB_A                         120
#define WIDTH_RICHEDIT_ORIGINAL                        60
#define RECORDERWND_BORDER_CIRCLE_RADIUS               8
#define VIDEO_MIN_RESOLUTION_SQRT                      4
#define RECORD_LIMIT_TIME                              3600

#endif  // RECORDER_SRC_UI_DEF_H_