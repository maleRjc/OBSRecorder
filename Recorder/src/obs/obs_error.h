#pragma once
#ifndef RECORDER_SRC_OBS_OBS_ERROR_H_
#define RECORDER_SRC_OBS_OBS_ERROR_H_


#ifdef _DEBUG

#define ERROR_TITLE "Tip"

#define INITIALIZE_LIBOBS_ERROR "Failed to initialize libobs."
#define INITIALIZE_AUDIO_ERROR "Failed to initialize audio."

#define NOT_FOUND_MODULE_ERROR \
  "Failed to initialize video:  Graphics module not found."
#define INVALID_PARAMETERS_ERROR \
  "Failed to initialize video:  Invalid parameters."
#define UNSUPPORTED_ERROR                                               \
  "Failed to initialize video:\n\nRequired graphics API functionality " \
  "not found.  Your GPU may not be supported."
#define UNKNOWN_ERROR                                            \
  "Failed to initialize video.  Your GPU may not be supported, " \
  "or your graphics drivers may need to be updated."

#define LOW_DISK_WHEN_RECORDING_ERROR \
  "Recording stopped because of low disk space."
#define LOW_DISK_ERROR "Low disk space."

#define TIMEOUT_LIMITED_ERROR "Recording stopped because time out limited."

#else

#define ERROR_TITLE "��ʾ"

#define INITIALIZE_LIBOBS_ERROR "��ʼ��������ʧ�ܡ�"
#define INITIALIZE_AUDIO_ERROR "��ʼ����Ƶʧ�ܡ�"

#define NOT_FOUND_MODULE_ERROR "��ʼ����Ƶʧ�ܣ�û���ҵ�ͼ��ģ�顣"
#define INVALID_PARAMETERS_ERROR "��ʼ����Ƶʧ�ܣ���Ч������"
#define UNSUPPORTED_ERROR                                    \
  "��ʼ����Ƶʧ�ܣ�δ�ҵ�ͼ�ι��ܽӿڡ�\n" \
  "GPU���ܲ�֧�ִ˹��ܡ�"
#define UNKNOWN_ERROR                                  \
  "��ʼ����Ƶʧ�ܣ�GPU��֧�ִ˹��ܡ�\n" \
  "�����豸������Ҫ���¡�"

#define LOW_DISK_WHEN_RECORDING_ERROR "���̿ռ䲻�㣬��ֹͣ¼�ơ�"
#define LOW_DISK_ERROR "���̿ռ䲻�㡣"

#define TIMEOUT_LIMITED_ERROR "¼�Ƴ�ʱ������ʱ��Ϊ1Сʱ��"

#endif


#endif  // RECORDER_SRC_OBS_OBS_ERROR_H_