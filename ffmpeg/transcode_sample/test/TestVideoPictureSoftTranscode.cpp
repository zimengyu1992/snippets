#include <iostream>

using namespace std;

#include "StreamReader.h"
#include "VideoTranscode.h"
#include "VideoPictureSoftTranscode.h"

int main(int argc, char *argv[])
{
    cout << "transcode" << endl;

    // av_log_set_level(AV_LOG_DEBUG);
    av_log_set_level(AV_LOG_INFO);
    AVCodecParameters *videoCodecpar;
    AVCodecParameters *audioCodecpar;
    StreamReader reader;
    reader.setDataSource("rtmp://127.0.0.1:1935/live/Test", "flv");
    reader.open();

    VideoPictureSoftTranscode transcode;

    VideoDecodeParam decodeParam;
    decodeParam.setCodecId(AV_CODEC_ID_H264);
    transcode.setDecodeParam(decodeParam);

    VideoEncodeParam encodeParam;
    encodeParam.setEncodeMode(VIDEO_ENCODE_PICTURE);
    encodeParam.setEncodeIFrame(true);

    encodeParam.setCodecId(AV_CODEC_ID_MJPEG);
    encodeParam.setBitrate(2000);
    encodeParam.setVideoSize(VideoSize{352, 288});
    encodeParam.setFramerate(25);
    encodeParam.setIFrameInterval(25);

    encodeParam.setPicInterval(100);
    transcode.setEncodeParam(encodeParam);

    transcode.open();

    AVPacket *packet;
    packet = av_packet_alloc();

    while (true)
    {
        av_init_packet(packet);
        if (reader.readPacket(packet) == 0)
        {
            if (packet->stream_index == reader.getVideoStream()->index)
            {
                transcode.sendPacket(packet);
                if (transcode.receivePacket(packet) == 0)
                {
                    printf("get pic id:%d pts:%lld.\n", packet->stream_index, packet->pts);
                    FILE *pTotalFile = fopen("preview.jpg", "wb");
                    fwrite(packet->data, 1, packet->size, pTotalFile);
                    fclose(pTotalFile);
                }
            }
        }

        av_packet_unref(packet);
    }

    av_packet_free(&packet);
    avcodec_parameters_free(&videoCodecpar);
    avcodec_parameters_free(&audioCodecpar);

    transcode.close();
    reader.close();
    return 0;
}