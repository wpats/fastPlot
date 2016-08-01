
#pragma once

#include <vector>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>

struct Buffer
{
  float * m_frequencyBuffer;
  float * m_powerBuffer;
  uint32_t m_capacity;
  time_t m_time;
  uint32_t m_size;
  Buffer(uint32_t capacity);
  bool Resize(uint32_t capacity);
  bool AddData(float frequency, float power);
  uint32_t size() {
    return m_size;
  }
  void SetTime(time_t time) {
    this->m_time = time;
  }
};

class DataReader
{
  const char * m_fileName;
  FILE * m_inputFile;
  Buffer * m_buffer;
  time_t m_nextStartTime;
  bool m_done;
 private:
  time_t StringToTime(char * timeBuffer, uint32_t length);
 public:
  DataReader(const char * fileName);
  DataReader(FILE * file);
  void Initialize(FILE * file);
  int GetNext(Buffer * & buffer);
  bool IsDone() {
    return this->m_done;
  }
  bool Reset() {
    Buffer * dummy;
    this->m_done = false;
    return this->SeekTo(0) && this->GetNext(dummy);
  }
  bool SeekTo(uint32_t offset) {
    return !fseek(this->m_inputFile, offset, SEEK_SET);
  }
};

class DataSource
{
  DataReader m_dataReader;
  double m_startFrequency;
  double m_stopFrequency;
  uint32_t m_sampleRate;
  int m_fftSize;
  std::vector<std::pair<long, time_t>> m_index;
  bool SeekTo(off_t fftSampleOffset);
 public:
  DataSource(FILE * file, 
             double startFrequency, 
             double stopFrequency, 
             double sampleRate,
             int fftSize);
  void Initialize(FILE * file);
  Buffer * GetData(off_t fftSampleOffset);
  void GetMagnitudeData(off_t fftSampleOffset, float * destination, int fftSize);
};
