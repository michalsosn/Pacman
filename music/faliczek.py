#!/usr/bin/python

import sys
import argparse as ap
import struct as st


def main():
    parser = ap.ArgumentParser()
    parser.add_argument('file', type=ap.FileType('rb'), 
                        help='the wave file to process')
    parser.add_argument('-o', '--out', type=ap.FileType('w'), default=sys.stdout, 
                        help='place the output into file')

    args = parser.parse_args()

    def comment(output=''):
        print(' * ' + output, file=args.out)

    print('/**', file=args.out)
    comment()
    comment('Generated with faliczek.py, by Michał Sośnicki')
    comment('I learned about wave file format from:') 
    comment('http://pratikmhatre.wordpress.com/2012/11/05/play-wav-file-using-dac-on-lpc2148/')
    comment()

    comment('File {}'.format(args.file.name))

    # < in unpack - little endian
    chunk_id, chunk_size, format = st.unpack('<4si4s', args.file.read(12))

    if chunk_id != b'RIFF' and format != b'WAVE': 
        raise ValueError('The opened file is not a wave file')

    comment('4 ChunkID = {} - Contains the letters “RIFF”.'.format(chunk_id))
    comment('4 ChunkSize = {} - Size of the file in bytes minus 8'
            .format(chunk_size))
    comment('4 Format = {} -  Contains the letters “WAVE”.'.format(format))
    comment()

    sub1_id, sub1_size, audio_format, \
    num_channels, sample_rate, byte_rate, \
    block_align, bits_per_sample = st.unpack('<4sihhiihh', args.file.read(24)) 

    if audio_format != 1: 
        raise ValueError('Warning: Audio_format = {}, file is compressed'
                        .format(audio_format))

    comment('4 Subchunk1ID = {} - Contains the letters “fmt”.'.format(sub1_id))
    comment('4 Subchunk1Size = {} - Size of the rest of the Subchunk.'
            .format(sub1_size))
    comment('2 AudioFormat = {} - Other than 1 indicate form of compression.'
            .format(audio_format))
    comment('2 NumChannels = {} - Mono = 1, Stereo = 2, etc.'.format(num_channels))
    comment('4 SampleRate = {} - 8000, 44100, etc.'.format(sample_rate))
    comment('4 ByteRate = {} - SampleRate * NumChannels * BitsPerSample/8'
            .format(byte_rate))
    comment('2 BlockAlign = {} - NumChannels * BitsPerSample/8'.format(block_align))
    comment('2 BitsPerSample = {} - 8 bits = 8, 16 bits = 16, etc.'
            .format(bits_per_sample))
    comment()

    sub2_id, sub2_size = st.unpack('<4si', args.file.read(8))
    comment('4 Subchunk2ID = {} - Contains the letters “data”.'.format(sub2_id))
    comment('4 Subchunk2Size = {} - NumSamples * NumChannels * BitsPerSample/8'
            .format(sub2_size))
    comment('                          This is the number of bytes in the data.')
    comment()

    print(' **/', file=args.out)

    sound_name = args.file.name[:args.file.name.find('.')] + 'Sound'
    print('\nconst char {}[] = {{'.format(sound_name), file=args.out)

    def read_bytes(input):
        byte = input.read(1)
        while byte:
            yield byte
            byte = input.read(1)

    IN_LINE = 20

    for i, byte in enumerate(read_bytes(args.file)):
        if i:
            args.out.write(', ')
        if not i % IN_LINE and i:
            args.out.write('\n')
        val, = st.unpack('B', byte)
        args.out.write('{}'.format(val))

    print('\n};', file=args.out)

    print('// total size: {}'.format(i + 1))


if __name__ == '__main__':
    main()
