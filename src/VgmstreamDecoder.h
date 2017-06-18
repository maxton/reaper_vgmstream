#pragma once
#include "reaper_plugin.h"
#include "reaper_plugin_functions.h"
#include "defines.h"

extern "C" {
#include <vgmstream.h>
}

class VgmstreamDecoder :
  public ISimpleMediaDecoder
{
public:
  VgmstreamDecoder();
  ~VgmstreamDecoder();
  ISimpleMediaDecoder *Duplicate();
  void Open(const char *filename, int diskreadmode, int diskreadbs, int diskreadnb);
  void Close(bool fullClose);
  void GetInfoString(char *buf, int buflen, char *title, int titlelen);

  const char *GetFileName() { return m_filename ? m_filename : ""; }
  const char *GetType() { return "VGMSTREAM"; }

  bool IsOpen() { return m_vgmstream != 0; }
  int GetNumChannels() { return !IsOpen() ? 0 : m_vgmstream->channels; }
  int GetBitsPerSample() { return 64; }
  double GetSampleRate() { return !IsOpen() ? 1 : m_vgmstream->sample_rate; }
  INT64 GetLength() { return !IsOpen() ? 1 : m_vgmstream->num_samples; }
  INT64 GetPosition() { return current_sample; }
  void SetPosition(INT64 pos);
  int ReadSamples(ReaSample *buf, int length);

  double GetLastDecodedTime() { return (double)GetPosition() / GetSampleRate(); }
private:
  void Reset();
  // Returns 0 if successful.
  int Open();
  VGMSTREAM* m_vgmstream;
  sample* readbuf;
  size_t readbuf_size;
  int readbuf_samples;
  int readbuf_sampleframes;
  int current_sample;
  // filename
  char *m_filename;
};

