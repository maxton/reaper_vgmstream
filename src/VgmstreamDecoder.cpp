#include "VgmstreamDecoder.h"

VgmstreamDecoder::VgmstreamDecoder() : m_filename(0), m_vgmstream(0)
{
  DEBUGPRINTF("VgmstreamDecoder::VgmstreamDecoder()" NL);
  this->Reset();
}

void VgmstreamDecoder::Open(const char *filename, int diskreadmode, int diskreadbs, int diskreadnb)
{
  DEBUGPRINTF("VgmstreamDecoder::Open(filename, diskreadmode, ...)" NL);
  DEBUGPRINTF("Given: %s " NL, filename ? filename : "");
  DEBUGPRINTF("Existing: %s" NL, m_filename ? m_filename : "");
  if (filename == 0) {
    if (m_filename == 0) {
      DEBUGPRINTF("No filenames exist..." NL);
      return;
    }
    else {
      DEBUGPRINTF("Reusing old filename" NL);
    }
  }
  else {
    DEBUGPRINTF("Using new filename" NL);
    free(m_filename);
    m_filename = _strdup(filename);
  }
  Open();
}

int VgmstreamDecoder::Open()
{
  DEBUGPRINTF("VgmstreamDecoder::Open()" NL);
  if (m_vgmstream) {
    DEBUGLOGLN("VgmstreamDecoder::Open(): This is already open so let's reset.");
    this->Reset();
  }
  if (m_filename == 0) {
    DEBUGLOGLN("VgmstreamDecoder::Open(): Null filename");
    return -1;
  }
  m_vgmstream = init_vgmstream(m_filename);

  if (!m_vgmstream) {
    DEBUGLOG("VgmstreamDecoder::Open(): OPEN failed");
    return -1;
  }
  readbuf_size = 1024 * sizeof(sample) * m_vgmstream->channels;
  if (readbuf) free(readbuf);
  readbuf_samples = readbuf_size / sizeof(sample);
  readbuf_sampleframes = readbuf_samples / m_vgmstream->channels;
  readbuf = (sample*)malloc(readbuf_size);
  current_sample = 0;
  DEBUGPRINTF("VgmstreamDecoder::Open(): Open successfully" NL);
  return 0;
}

void VgmstreamDecoder::Close(bool)
{
  DEBUGPRINTF("VgmstreamDecoder::Close" NL);
  Reset();
}

void VgmstreamDecoder::Reset()
{
  DEBUGPRINTF("VgmstreamDecoder::Reset" NL);
  close_vgmstream(m_vgmstream);
  m_vgmstream = 0;
  if(readbuf) free(readbuf);
  readbuf = 0;
}

VgmstreamDecoder::~VgmstreamDecoder()
{
  DEBUGPRINTF("VgmstreamDecoder::~VgmstreamDecoder" NL);
  this->Reset();
  if (m_filename) free(m_filename);
}

ISimpleMediaDecoder* VgmstreamDecoder::Duplicate()
{
  DEBUGPRINTF("VgmstreamDecoder::Duplicate" NL);
  VgmstreamDecoder *r = new VgmstreamDecoder;
  r->Open(this->m_filename, 0, 0, 0);
  return r;
}

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
int VgmstreamDecoder::ReadSamples(ReaSample *buf, int length){
  if (!m_vgmstream) return 0;

  int total_read = 0;
  int true_length = length * m_vgmstream->channels;
  do {
    int read_size = MIN(readbuf_samples, true_length);
    render_vgmstream(readbuf, read_size/m_vgmstream->channels, m_vgmstream);
    for (int x = 0; x < read_size; x++) {
      *(buf++) = readbuf[x] / 32768.0;
    }
    total_read += read_size;
    true_length -= read_size;
  } while (true_length > 0);
  int ret = total_read/m_vgmstream->channels;
  current_sample += ret;
  return ret;
}

// This is probably SLOW because VGMstream does not allow nonlinear or backward seeking.
void VgmstreamDecoder::SetPosition(INT64 pos) {
  if (!m_vgmstream || pos > GetLength()) return;

  if (pos < current_sample) {
    reset_vgmstream(m_vgmstream);
    current_sample = 0;
  }

  int samples_needed = (int)pos - current_sample;
  while(samples_needed > 0) {
    int read_size = MIN(readbuf_sampleframes, samples_needed);
    render_vgmstream(readbuf, read_size, m_vgmstream);
    samples_needed -= read_size;
  }
  current_sample = (int)pos;
}

void VgmstreamDecoder::GetInfoString(char *buf, int buflen, char *title, int titlelen)
{
  lstrcpyn(title, "File Properties", titlelen);
  if (IsOpen())
  {
    char temp[4096];
    // TODO: Put codec info here.
    sprintf_s(temp, 
      "Samplerate: %.0f" NL
      "Channels: %d" NL,
      GetSampleRate(), GetNumChannels());

    lstrcpyn(buf, temp, buflen);
  }
  else
    strcpy_s(buf, buflen, "Error: File not has been opened succesfully");
}