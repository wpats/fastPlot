#include <vector>

class CircularBuffer 
{
  uint32_t m_bufferCount;
  uint32_t m_currentBufferIndex;
 public:
  typedef std::vector<std::pair<double, double> > BufferType;
  typedef std::list<BufferType *> BufferListType;
 private:
  std::vector<BufferType> m_buffers;
 public:
  CircularBuffer(uint32_t bufferCount)
    : m_bufferCount(bufferCount),
      m_currentBufferIndex(0),
      m_buffers(bufferCount)
      {
      }
  
  void appendPoint(double frequency, double power) {
    this->m_buffers[this->m_currentBufferIndex].push_back(std::make_pair(frequency, power));
  }

  void nextBuffer() {
    this->m_currentBufferIndex = (this->m_currentBufferIndex + 1) % this->m_buffers.size();
    this->m_buffers[this->m_currentBufferIndex].resize(0);
  }

  BufferListType getBuffers() {
    // Return buffers and sizes in order of oldest to newest
    BufferListType resultList;
    for (uint32_t i = 0; i < this->m_buffers.size(); i++) {
      uint32_t index = (i + this->m_currentBufferIndex + 1) % this->m_buffers.size();
      resultList.push_back(&this->m_buffers[index]);
    }
    return resultList;
  }
};

