#pragma once

#include <vector>

class DelayLine {
public:
    void init(size_t size)
    {
        buffer.resize(size);
        writePos = 0;
    }

    void clear()
    {
        if (buffer.size())
            std::fill(buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
    }

    inline size_t getBufferSize() const
    {
        return buffer.size();
    }

    void write(float sample)
    {
        if (!buffer.size())
            return;

        buffer[writePos] = sample;
        writePos++;
        if (writePos >= (int)getBufferSize()) {
            writePos = 0;
        }
    }

    float read(float delayInSamples) const 
    {
        if (!buffer.size())
            return 0.0f;

        float readPos;
        int readPosInt;
        float frac;
        float sample1, sample2;
        
        readPos = (float)writePos - delayInSamples;
        while (readPos < 0.0f) {
            readPos += (float)getBufferSize();
        }
        while (readPos >= (float)getBufferSize()) {
            readPos -= (float)getBufferSize();
        }
        
        readPosInt = (int)readPos;
        frac = readPos - (float)readPosInt;
        
        sample1 = buffer[readPosInt];
        readPosInt++;
        if (readPosInt >= getBufferSize()) {
            readPosInt = 0;
        }
        sample2 = buffer[readPosInt];
        
        /* Linear interpolation */
        return sample1 + frac * (sample2 - sample1);
    }

private:
    std::vector<float> buffer;
    int writePos;
};
