#include "FrameFilter.h"

FrameFilter::FrameFilter()
{
    pFilterGraph = nullptr;
    pBuffersrcCtx = nullptr;
    pBuffersinkCtx = nullptr;
}

FrameFilter::~FrameFilter()
{
    if (pBuffersrcCtx != nullptr)
    {
        avfilter_free(pBuffersrcCtx);
    }
    if (pBuffersrcCtx != nullptr)
    {
        avfilter_free(pBuffersinkCtx);
    }
    if (pFilterGraph != nullptr)
    {
        avfilter_graph_free(&pFilterGraph);
    }
}

bool FrameFilter::InitFrameFilter(int videoWidth, int videoHeight, AVPixelFormat videoFmt, const char* filtersDesc)
{
    char args[512];
    int ret = 0;
    //缓存输入和缓存输出
    const AVFilter* buffersrc = avfilter_get_by_name("buffer");
    const AVFilter* buffersink = avfilter_get_by_name("buffersink");

    //创建输入输出参数
    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();

    enum AVPixelFormat pix_fmts[] = { videoFmt, videoFmt };

    //创建滤镜容器
    pFilterGraph = avfilter_graph_alloc();
    if (!outputs || !inputs || !pFilterGraph)
    {
        ret = AVERROR(ENOMEM);
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        return false;
    }

    //初始化数据帧的格式
    sprintf_s(args, sizeof(args),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        videoWidth, videoHeight, videoFmt,
        1, 1200000,
        0, 1);

    //输入数据缓存
    ret = avfilter_graph_create_filter(&pBuffersrcCtx, buffersrc, "in", args, NULL, pFilterGraph);
    if (ret < 0)
    {
        printf("Cannot create buffer source");
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        return false;
    }

    //输出数据缓存
    ret = avfilter_graph_create_filter(&pBuffersinkCtx, buffersink, "out", NULL, NULL, pFilterGraph);
    if (ret < 0)
    {
        printf("Cannot create buffer sink");
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        return false;
    }

    //设置sink 输出的pixel format
    ret = av_opt_set_int_list(pBuffersinkCtx, "pix_fmts", pix_fmts, videoFmt, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        return false;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
    */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
    */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = pBuffersrcCtx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
    */
    inputs->name = av_strdup("out");
    inputs->filter_ctx = pBuffersinkCtx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    //char filter_desc[200] = {0};
    //snprintf(filter_desc, sizeof(filter_desc), "scale=%d:%d,transpose=cclock",videoHeight,videoWidth);

    //初始化滤镜
    if ((ret = avfilter_graph_parse_ptr(pFilterGraph, filtersDesc, &inputs, &outputs, NULL)) < 0) {
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        return false;
    }
       

    //滤镜生效
    if ((ret = avfilter_graph_config(pFilterGraph, NULL)) < 0) {
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        return false;
    }

	return true;
}

bool FrameFilter::ConvertFrame(AVFrame* frameSource, AVFrame* frameTarget)
{
    // push the decoded frame into the filtergraph
    if (av_buffersrc_add_frame_flags(pBuffersrcCtx, frameSource, AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
    {
        printf("Error while feeding the filtergraph\n");
        return false;
    }

    // pull filtered frames from the filtergraph
    if (av_buffersink_get_frame(pBuffersinkCtx, frameTarget) < 0)
    {
        printf("Error while geting the filtergraph\n");
        return false;
    }

    return true;
}
