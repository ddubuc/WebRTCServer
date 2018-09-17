#ifndef OMEKASHI_OPENCVUTIL_H
#define OMEKASHI_OPENCVUTIL_H

#include "opencv2/opencv.hpp"

/**------------------------------------------------------------*
* @fn          OpenCVのピクチャーインピクチャ http://qiita.com/kinojp/items/d2d9a68a962b34b62888
* @brief       画像内に画像を貼り付ける（位置を座標で指定）
* @par[in ]    srcImg  背景画像
* @par[in ]    smallImg    前景画像
* @par[in ]    p0  前景画像の左上座標
* @par[in ]    p1  前景画像の右下座標
*------------------------------------------------------------*/
void drawPinP(cv::Mat& dstImg, const cv::Mat &srcImg, const cv::Mat &smallImg, const cv::Point2f p0, const cv::Point2f p1);

/**-----------------------------------------------------------*
 * @fn  DrawTransPinP http://kurino.xii.jp/Gallery/Computer/Code/ocv/transparent.html
 * @brief   透過画像を重ねて描画する
 * @param[out] img_dst
 * @param[in ] transImg 前景画像。アルファチャンネル付きであること(CV_8UC4)
 * @param[in ] baseImg  背景画像。アルファチャンネル不要(CV_8UC4)
 *------------------------------------------------------------*/
void drawTransPinP(cv::Mat &img_dst, const cv::Mat transImg, const cv::Mat baseImg, std::vector<cv::Point2f> tgtPt);

#endif // OMEKASHI_OPENCVUTIL_H
