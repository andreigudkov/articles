#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include <iostream>
#include <exception>
#include <algorithm>

#define ensure(expr) \
if (!(expr)) { \
  std::ostringstream str; \
  str << "Assertion [" << #expr << "] failed in [" << __FUNCTION__ << "]"; \
  throw std::runtime_error(str.str().c_str()); \
}

cv::Mat load(const std::string& path) {
  cv::Mat img = cv::imread(path, cv::IMREAD_UNCHANGED);
  ensure(img.cols > 0);
  ensure(img.rows > 0);
  ensure(img.depth() == CV_8U);
  if (img.channels() == 1) {
    img = cv::imread(path, cv::IMREAD_COLOR);
  } else if (img.channels() == 3) {
    // do nothing
  } else if (img.channels() == 4) {
    // merge with white bg
    cv::Mat tmp;
    img.convertTo(tmp, CV_32FC4, 1.0/255.0);

    std::array<cv::Mat, 4> channels;
    cv::split(tmp, channels.begin());

    cv::Mat bg(img.rows, img.cols, CV_32FC1, cv::Scalar(1.0));
    for (int i = 0; i < 3; i++) {
      channels[i] = channels[i].mul(channels[3]) + bg.mul(1.0 - channels[3]);
    }

    cv::merge(channels.begin(), 3, tmp);

    tmp.convertTo(img, CV_8UC3, 255);
  } else {
    ensure(false);
  }
  return img;
}

cv::Mat downscale(cv::Mat img, double ratio) {
  ensure(ratio < 1.0);
  while (ratio < 0.5) {
    img = cv::Mat(img, cv::Rect(0, 0, img.cols/2*2, img.rows/2*2));
    cv::Mat tmp;
    cv::resize(img, tmp, cv::Size(img.cols/2, img.rows/2), 0, 0, cv::INTER_LINEAR);
    img = tmp;
    ratio = ratio * 2.0;
  }
  if (img.cols * ratio != img.cols) {
    img = cv::Mat(img, cv::Rect(0, 0, img.cols/2*2, img.rows/2*2));
    cv::Mat tmp; 
    cv::resize(img, tmp, cv::Size(img.cols*ratio, img.rows*ratio), 0, 0, cv::INTER_CUBIC);
    img = tmp;
  }
  return img;
}

void npass() {
  cv::Mat src = load("npass_original.jpg");

  {
    cv::Mat dst;
    cv::resize(src, dst, cv::Size(src.cols * 0.125, src.rows * 0.125), 0, 0, cv::INTER_CUBIC);
    cv::imwrite("npass_single.jpg", dst);
  }

  {
    cv::Mat dst = downscale(src, 0.125);
    cv::imwrite("npass_multi.jpg", dst);
  }
}

void usm() {
  cv::Mat img = load("usm_original.png");
  img = downscale(img, 0.125);

  {
    cv::imwrite("usm_1.png", img);
  }

  {
    cv::Mat dst;
    cv::GaussianBlur(img, dst, cv::Size(0, 0), 2.0, 0.0, cv::BORDER_DEFAULT);
    cv::addWeighted(img, 1.5, dst, -0.5, 0, dst);
    cv::imwrite("usm_2.png", dst);
  }
}

void quality() {
  auto save_frame = [](cv::Mat img, int q, const std::string& path) {
    cv::Mat tmp = img.clone();
    std::ostringstream str;
    str << "Q=" << q;
    double coef = img.rows / 256.0;
    cv::putText(tmp, str.str(),
                cv::Point(10*coef, tmp.rows-10*coef), // bottom left
                cv::FONT_HERSHEY_PLAIN,     // font
                coef,                       // scale
                cv::Scalar::all(0),         // color
                1,                          // thickness
                cv::LINE_8);
    cv::imwrite(path, tmp, {cv::IMWRITE_JPEG_QUALITY, q});
  };

  auto save_anim = [&save_frame](cv::Mat img, const std::string& path) {
    std::ostringstream str;
    str << "convert";
    str << " -background white";
    str << " -delay 100";
    for (int q : {100, 75}) {
      const std::string tmp = "/tmp/q" + std::to_string(q) + ".jpg";
      save_frame(img, q, tmp);
      str << ' ' << tmp;
    }
    str << ' ' << path;
    ensure(system(str.str().c_str()) == 0);
  };

  {
    cv::Mat img = load("qual_original.png");
    img = downscale(img, 0.25);
    save_anim(img, "qual_big.gif");
  }
  {
    cv::Mat img = load("qual_original.png");
    img = downscale(img, 0.125);
    save_anim(img, "qual_small.gif");
  }

  auto normalize = [](cv::Mat img) {
    std::vector<float> values;
    values.reserve(img.rows * img.cols);

    for (int i = 0; i < img.rows; i++) {
      for (int j = 0; j < img.cols; j++) {
        values.push_back(img.at<float>(i, j));
      }
    }

    std::sort(values.begin(), values.end());

    double threshold = values[values.size() * 0.98];

    for (int i = 0; i < img.rows; i++) {
      for (int j = 0; j < img.cols; j++) {
        if (img.at<float>(i, j) > threshold) {
          img.at<float>(i, j) = threshold;
        } else {
          img.at<float>(i, j) = img.at<float>(i, j) / threshold;
        }
      }
    }
  };

  {
    cv::Mat img = cv::imread("qual_original.png", cv::IMREAD_GRAYSCALE);
    img = downscale(img, 0.2);
    img = cv::Mat(img, cv::Rect(0, 0, img.cols/2*2, img.rows/2*2));

    cv::Mat tmp;
    img.convertTo(tmp, CV_32FC1, 1.0/255.0);
    img = tmp;

    cv::Mat dct;
    cv::dct(img, dct, 0);

    cv::Mat pos(dct.size(), dct.type(), 0.0f);
    cv::Mat neg(dct.size(), dct.type(), 0.0f);
    cv::Mat zero(dct.size(), dct.type(), 0.0f);
    for (int i = 0; i < dct.rows; i++) {
      for (int j = 0; j < dct.cols; j++) {
        if (dct.at<float>(i, j) >= 0.0) {
          pos.at<float>(i, j) = dct.at<float>(i, j);
        } else {
          neg.at<float>(i, j) = -dct.at<float>(i, j);
        }
      }
    }
    normalize(pos);
    normalize(neg);

    std::array<cv::Mat, 3> channels{{neg, zero, pos}};
    cv::merge(channels.begin(), 3, dct);

    dct.convertTo(tmp, CV_8UC3, 255);
    dct = tmp;

    cv::imwrite("qual_dct.png", dct);
  }
}

void crop() {
  auto find_faces = [](cv::Mat img){
    cv::CascadeClassifier fdt;
    fdt.load("/home/gudok/SVN/walls/haarcascade_frontalface_alt.xml");
    std::vector<cv::Rect> faces;
    fdt.detectMultiScale(img, faces, 1.5, 2, 0|CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
    return faces;
  };

  auto crop_smart = [](cv::Mat img, const std::vector<cv::Rect>& faces){
    ensure(img.cols > img.rows);
    int width = img.rows;
    int best_x = 0;
    double best_score = 0.0;
    for (int x = 0; x < img.cols - width; x++) {
      double score = 0.0;
      for (const cv::Rect& face : faces) {
        if (face.x >= x && face.x + face.width < x + width) {
          double area_score = face.width * face.height;
          double pos_score = 1.0 / std::sqrt(
            std::pow((x+width/2) - (face.x+face.width/2), 2.0) +
            std::pow((0+img.rows/2) - (face.y+face.height/2), 2.0)
          );
          score += area_score * pos_score;
        }
      }
      if (score > best_score) {
        best_x = x;
        best_score = score;
      }
    }
    return cv::Mat(img, cv::Rect(best_x, 0, width, img.rows));
  };


  auto crop_center = [](cv::Mat img){
    if (img.cols < img.rows) {
      return cv::Mat(img, cv::Rect(0, (img.rows-img.cols)/2, img.cols, img.cols));
    } else {
      return cv::Mat(img, cv::Rect((img.cols-img.rows)/2, 0, img.rows, img.rows));
    }
  };

  auto downscale_faces = [](const std::vector<cv::Rect>& faces, double scale) {
    std::vector<cv::Rect> res;
    for (size_t i = 0; i < faces.size(); i++) {
      res.push_back(cv::Rect(
        faces[i].x * scale,
        faces[i].y * scale,
        faces[i].width * scale,
        faces[i].height * scale
      ));
    }
    return res;
  };

  auto mark_faces = [](cv::Mat img, const std::vector<cv::Rect>& faces){
    img = img.clone();
    for (size_t i = 0; i < faces.size(); i++) {
      cv::rectangle(img, faces[i], cv::Scalar(0, 0, 255));
    }
    return img;
  };

  cv::Mat img = load("crop_original.jpg");
  auto faces = find_faces(img);
  cv::Mat full = mark_faces(downscale(img, 0.4), downscale_faces(faces, 0.4));
  cv::Mat center = downscale(crop_center(img), 0.25);
  cv::Mat smart = downscale(crop_smart(img, faces), 0.25);

  cv::imwrite("crop_full.jpg", full);

  cv::Mat panno(smart.rows, center.cols + 15 + smart.cols, CV_8UC3, cv::Scalar(255, 255, 255));
  center.copyTo(cv::Mat(panno, cv::Rect(0, 0, center.cols, center.rows)));
  smart.copyTo(cv::Mat(panno, cv::Rect(panno.cols - smart.cols, panno.rows - smart.rows, smart.cols, smart.rows)));
  cv::imwrite("crop_thumbs.jpg", panno);
}

int main(int argc, char** argv) {
  npass();
  usm();
  quality();
  crop();

  return 0;
}

