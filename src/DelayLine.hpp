#ifndef DELAY_LINE_HPP_INCLUDED
#define DELAY_LINE_HPP_INCLUDED

#include <vector>

/**
 * @file DelayLine.hpp
 * @brief Simple circular delay line with linear interpolation.
 */

/**
 * @class DelayLine
 * @brief Circular-buffer delay line supporting fractional read-out.
 *
 * The delay line writes samples into a circular buffer and reads them back
 * at an arbitrary fractional delay using linear interpolation. It is used
 * once per stereo channel in the flanger.
 */
class DelayLine {
public:
    /**
     * @brief Allocates and initializes the delay-line buffer.
     * @param size Desired buffer length in samples.
     *
     * Resizes the internal buffer to @p size and resets the write pointer.
     * Existing contents are zero-initialized by @c std::vector.
     */
    void init(size_t size)
    {
        buffer.resize(size);
        writePos = 0;
    }

    /**
     * @brief Clears the delay-line buffer and resets the write pointer.
     *
     * Sets every sample to zero. Safe to call even if the buffer has not
     * been allocated yet.
     */
    void clear()
    {
        if (buffer.size())
            std::fill(buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
    }

    /**
     * @brief Returns the allocated buffer length in samples.
     * @return Buffer size in samples.
     */
    inline size_t getBufferSize() const
    {
        return buffer.size();
    }

    /**
     * @brief Writes a single sample into the circular buffer.
     * @param sample Sample value to store.
     *
     * The write pointer is advanced and wrapped automatically.
     */
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

    /**
     * @brief Reads a sample from the delay line at a fractional delay.
     * @param delayInSamples Delay time in samples (may be fractional).
     * @return Interpolated sample value.
     *
     * The read position is computed behind the current write pointer by
     * @p delayInSamples samples and wrapped into the valid buffer range.
     * Linear interpolation is applied between the two surrounding samples.
     */
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
    std::vector<float> buffer;   ///< Circular buffer storing delayed samples.
    int writePos;                ///< Current write position in samples.
};

#endif // DELAY_LINE_HPP_INCLUDED
