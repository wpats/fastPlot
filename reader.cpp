#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <cmath>
#include <algorithm>
#include <random>
#include "reader.h"

Buffer::Buffer(uint32_t capacity)
  : m_capacity(capacity),
    m_time(0),
    m_size(0)
{
  this->m_frequencyBuffer = new float[capacity];
  this->m_powerBuffer = new float[capacity];
}

bool Buffer::Resize(uint32_t capacity)
{
  if (capacity > this->m_capacity) {
    float * tmp_frequency = new float[capacity];
    float * tmp_power = new float[capacity];
    memcpy(tmp_frequency, this->m_frequencyBuffer, this->m_capacity * sizeof(float));
    memcpy(tmp_power, this->m_powerBuffer, this->m_capacity * sizeof(float));
    this->m_frequencyBuffer = tmp_frequency;
    this->m_powerBuffer = tmp_power;
    this->m_capacity = capacity;
    return true;
  }
  return false;
}


bool Buffer::AddData(float frequency, float power)
{
  if (this->size() == this->m_capacity) {
    if (!this->Resize(2 * this->m_capacity)) {
      return false;
    }
  }
  this->m_frequencyBuffer[this->size()] = frequency;
  this->m_powerBuffer[this->size()] = power;
  this->m_size++;
  return true;
}

DataReader::DataReader(FILE * file)
  : m_inputFile(file),
    m_buffer(NULL),
    m_done(false)
{
  if (file != NULL) {
    this->Initialize(file);
  }
}

DataReader::DataReader(const char * fileName)
  : DataReader(fopen(fileName, "r"))
{
  this->m_fileName = fileName;
}

void DataReader::Initialize(FILE * file)
{
  assert(file != NULL);
  this->m_inputFile = file;
  this->m_buffer = new Buffer(1024);
  if (this->m_buffer == NULL) {
    fprintf(stderr, "Failed to allocate buffer, exiting...\n");
    exit(-1);
  }
  this->Reset();
}

time_t DataReader::StringToTime(char * timeBuffer, uint32_t length)
{
  const char timeformat[] = "%Y%m%d-%T";
  struct tm timeStruct;
  memset(&timeStruct, 0, sizeof(struct tm));
  if (strptime(timeBuffer, timeformat, &timeStruct) == NULL) {
    fprintf(stderr, "strptime returned null");
    exit(1);
  }
  time_t time;
  if ((time = mktime(&timeStruct)) == -1) {
    perror("mktime");
    exit(1);
  }
  return time;
}

void DataReader::TimeToString(time_t time, char * buffer, uint32_t length)
{
  const char timeformat[] = "%Y%m%d-%T";
  struct tm * timeStruct = localtime(&time);
  if (timeStruct == nullptr) {
    perror("localtime");
    exit(1);
  }
  if (strftime(buffer, length, timeformat, timeStruct) == 0) {
    fprintf(stderr, "strftime returned 0");
    exit(1);
  }
}

int DataReader::GetNext(Buffer * & buffer)
{
  size_t len = 1024;
  char line[len];
  char * bufferPtr = line;
  ssize_t read;
  if (this->IsDone()) {
    return -1;
  }
  this->m_buffer->m_size = 0;
  this->m_buffer->m_time = this->m_nextStartTime;
  while ((read = getline(&bufferPtr, &len, this->m_inputFile)) != -1) {
    uint32_t frequency;
    float power;
    char time[128];
    if (sscanf(line, "freq %u power_db %f\n", &frequency, &power) == 2) {
      this->m_buffer->AddData(float(frequency), power);
    } else if (sscanf(line, "Start scan at %s\n", time) == 1) {
      this->m_nextStartTime = this->StringToTime(time, 128);
      break;
    }
  }
  buffer = this->m_buffer;
  if (read == -1) {
    this->m_done = true;
    return -1;
  }
  return 0;
}

DataSource::DataSource(FILE * file, 
                       double startFrequency, 
                       double stopFrequency, 
                       double sampleRate,
                       int fftSize)
  : m_dataReader(reinterpret_cast<FILE *>(NULL)),
    m_index(),
    m_startFrequency(startFrequency), 
    m_stopFrequency(stopFrequency),
    m_sampleRate(sampleRate),
    m_fftSize(fftSize)
{
  fprintf(stderr, "file[0x%p] startFrequency[%f] stopFrequency[%f] sampleRate[%f] fftSize[%d]\n",
          file, 
          startFrequency, 
          stopFrequency, 
          sampleRate,
          fftSize);
  fflush(stderr);
  if (file != NULL) {
    this->Initialize(file);
  }
}

void DataSource::Initialize(FILE * file)
{
  assert(file != NULL);
  fprintf(stderr, "file[%p]\n", file);
  fflush(stderr);
  this->m_dataReader.Initialize(file);
  Buffer * buffer = NULL;
  do {
    off_t offset = ftell(file);
    if (buffer != NULL) {
      this->m_index.back().second = buffer->m_time;
    }
    this->m_index.push_back(std::make_pair(offset, 0));
  } while (this->m_dataReader.GetNext(buffer) == 0);
  if (buffer != NULL) {
    this->m_index.back().second = buffer->m_time;
  }
  this->m_dataReader.Reset();
}

bool DataSource::SeekTo(off_t fftSampleOffset)
{
  off_t index = fftSampleOffset / this->m_fftSize;
  long offset = this->m_index.at(index).first;
  return this->m_dataReader.SeekTo(offset);
}

Buffer * DataSource::GetData(off_t fftSampleOffset)
{
  Buffer * buffer;
  this->SeekTo(fftSampleOffset);
  if (this->m_dataReader.GetNext(buffer)) {
    fprintf(stderr, "Error getting FFT data %ld\n", fftSampleOffset);
    exit(-1);
  }
  return buffer;
}

// Get the fft magnitudes appropriately decimated and padded.
//
void DataSource::GetMagnitudeData(off_t fftSampleOffset, float * destination, int fftSize)
{
#if 0
  std::random_device rd; //seed generator
  std::mt19937_64 generator{rd()}; //generator initialized with seed from rd
  std::uniform_real_distribution<float> dist{0.0, 10.0}; //the range is inclusive, so this produces numbers in range [0, 10), same as before
  for (int i = 0; i < fftSize; i++) {
    destination[i] = dist(generator);
  }

  return;
#endif

  Buffer * buffer = this->GetData(fftSampleOffset);

  // Sort by frequency.
  std::vector<int> indices(buffer->size());
  for (uint32_t i = 0; i < buffer->size(); i++) {
    indices[i] = i;
  }
  std::sort(indices.begin(),
            indices.end(),
            [=](int i, int j) -> bool 
            { return buffer->m_frequencyBuffer[i] < buffer->m_frequencyBuffer[j]; }
            );
  // Fill in the dest with appropriately padded magnitude data.
  float startFrequency = this->m_startFrequency;
  float outputBinSize = (this->m_stopFrequency - this->m_startFrequency)/fftSize;
  float error = outputBinSize/2;
  for (int i = 0; i < fftSize; i++) {
    destination[i] = -100.0;
  }
  for (uint32_t i = 0, bufferIndex = 0; i < uint32_t(fftSize);) {
    float outputFrequency = startFrequency + i * outputBinSize;
    if (bufferIndex < buffer->size()) {
      float inputFrequency = buffer->m_frequencyBuffer[indices[bufferIndex]];
      //fprintf(stderr, "inputFrequency[%f] outputFrequency[%f]\n", inputFrequency, outputFrequency);
      float thisError = std::abs(inputFrequency - outputFrequency);
      if (thisError < error) {
        float power = buffer->m_powerBuffer[indices[bufferIndex]];
        destination[i] += thisError/error * power;
        // i++;
        bufferIndex++;
      } else if (inputFrequency < outputFrequency) {
        bufferIndex++;
      } else {
        i++;
      }
    } else {
      i++;
    }
  }
}
