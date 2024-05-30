#include "common.h"
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include "native_opencv.h"
#include <vector>
#include <cstdint> // Include the C++ header for fixed-width integers
#include <map>
using json = nlohmann::json;
using namespace cv;
using namespace std;

extern "C"
{
    /*FUNCTION_ATTRIBUTE
    typedef struct
    {
        const char *key;
        const char *value;
    } struct_color;

    FUNCTION_ATTRIBUTE
    struct_color getColorsOutput(std::map<std::string, std::string> *colors_output);
    {
        // Create an array of MyStruct
        struct_color *arr = new struct_color[colors_output.size()];

        int i = 0;
        for (const auto &entry : colors_output)
        {
            arr[i].key = entry.first.c_str();
            arr[i].value = entry.second.c_str();
            i++;
        }

        return arr;
    }*/

    FUNCTION_ATTRIBUTE
    const char *opencvVersion()
    {
        return CV_VERSION;
    }

    FUNCTION_ATTRIBUTE
    int check_postive_or_negative(const cv::Mat &roi, const std::vector<std::vector<cv::Point>> &contours)
    {
        int height_of_area = roi.rows;
        int half_of_height = height_of_area / 2;
        int postive_or_invalid_num = 1;
        for (const auto &contour : contours)
        {
            double area = cv::contourArea(contour);
            cv::Rect bounding_rect = cv::boundingRect(contour);
            int y = bounding_rect.y;
            int h = bounding_rect.height;

            // cout << "heigh " << h << " width " << w << endl;
            if (h >= 12 && h <= 25)
            {
                if (y >= half_of_height)
                {
                    postive_or_invalid_num = -3;
                }
                else
                {
                    postive_or_invalid_num = 1;
                }
            }
        }
        return postive_or_invalid_num;
    }

    FUNCTION_ATTRIBUTE
    int line_count(const std::vector<std::vector<cv::Point>> &contours, const cv::Mat &roi)
    {
        int line_counts = 0;
        for (size_t i = 0; i < contours.size(); ++i)
        {
            cv::Moments moments = cv::moments(contours[i]);
            int area = moments.m00;
            int h = 2 * moments.m00 / cv::arcLength(contours[i], true);

            // cout << "heigh " << h << " width " << w << endl;
            if (h >= 12 && h <= 25)
            {
                line_counts++;
            }
        }
        int line_count = line_counts;
        if (line_counts == 1)
        {
            line_count = check_postive_or_negative(roi, contours);
        }
        return line_count;
    }

    FUNCTION_ATTRIBUTE
    bool find_valid_image(const cv::Mat &roi_image)
    {
        cv::Mat binary_mask;
        // cv::threshold(gray_inner_image, binary_mask, 128, 255, cv::THRESH_BINARY);
        //  Invert the binary mask to create a mask for other colors
        cv::Scalar lower_bound = {0, 0, 140};
        cv::Scalar upper_bound = {179, 70, 255};
        cv::Mat hsv_image;
        cv::cvtColor(roi_image, hsv_image, cv::COLOR_BGR2HSV);
        cv::inRange(hsv_image, lower_bound, upper_bound, binary_mask);
        double white_percentage = (cv::countNonZero(binary_mask) / static_cast<double>(binary_mask.size().area())) * 100;
        if (white_percentage >= 50)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    FUNCTION_ATTRIBUTE
    std::pair<cv::Mat, int> inner_line_detection(const cv::Mat &roi)
    {
        // first version
        /*cv::Mat gray, equalized_image, image;
        cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, equalized_image);

        // Adjust intensity levels to enhance dark and light regions
        double alpha = 1.5; // Controls the intensity of dark regions
        double beta = 50;   // Controls the intensity of light regions
        cv::convertScaleAbs(equalized_image, image, alpha, beta);

        // Apply adaptive thresholding to create a binary image
        cv::Mat binary_mask;
        cv::inRange(image, cv::Scalar(0), cv::Scalar(100), binary_mask);

        // Find contours in the binary mask
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        int line_counts = line_count(contours);

        // Draw contours on the original image (optional)
        cv::drawContours(roi, contours, -1, cv::Scalar(0, 255, 0), 2);

        return std::make_pair(roi, line_counts);*/
        cv::Mat gray, equalized_image, image;
        cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
        // double
        cv::GaussianBlur(gray, equalized_image, cv::Size(7, 7), 0);
        cv::threshold(equalized_image, image, 0, 255, cv::THRESH_OTSU);
        // cv::equalizeHist(gray, equalized_image);

        // Adjust intensity levels to enhance dark and light regions
        // double alpha = 1.5; // Controls the intensity of dark regions
        // double beta = 50;   // Controls the intensity of light regions
        // cv::convertScaleAbs(equalized_image, image, alpha, beta);

        // Apply adaptive thresholding to create a binary image
        // scv::Mat binary_mask;
        // cv::inRange(image, cv::Scalar(0), cv::Scalar(100), binary_mask);

        // Find contours in the binary mask
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        // int line_counts = line_count(contours);

        bool status = find_valid_image(roi);
        int line_counts = -2;
        // platform_log("Decode done in %dms\n", status);
        if (true)
        {
            line_counts = line_count(contours, roi);
            // cv::drawContours(roi, contours, -1, cv::Scalar(0, 255, 0), 2);
        }

        return std::make_pair(roi, line_counts);
        // Draw contours on the original image (optional)
        // cv::drawContours(roi, contours, -1, cv::Scalar(0, 255, 0), 2);

        // return std::make_pair(roi, line_counts);
    }

    FUNCTION_ATTRIBUTE
    cv::Mat insertImage(const cv::Mat &roiImage, int roiY, int roiX, int roiWidth, int roiHeight, cv::Mat &originalImage)
    {
        cv::Mat replacementImageResized;
        resize(roiImage, replacementImageResized, Size(roiWidth, roiHeight));

        // Replace the ROI in the source image with the replacement image
        replacementImageResized.copyTo(originalImage(Rect(roiX, roiY, roiWidth, roiHeight)));
        return originalImage;
    }

    FUNCTION_ATTRIBUTE
    double calculate_length(const vector<Point> &rectangle)
    {
        // Calculate Euclidean distance between two opposite corners of the rectangle
        double length = norm(rectangle[0] - rectangle[2]);
        return length;
    }

    FUNCTION_ATTRIBUTE
    cv::Mat get_crop_image(const cv::Mat &uncroped_image, int &width, int &height)
    {
        // cv::Point center(uncroped_image.cols / 2, uncroped_image.rows / 2);
        //  Define the width (w) and height (h) for cropping
        // Calculate the center of the image
        int centerX = uncroped_image.cols / 2;
        int centerY = uncroped_image.rows / 2;

        // Define the width (w) and height (h) for cropping
        int w = width;  // Replace with your desired width
        int h = height; // Replace with your desired height

        // Calculate the top-left corner coordinates for cropping
        int x = centerX - w / 2;
        int y = centerY - h / 2;

        cv::Rect roi(x, y, w, h);
        cv::Mat croppedImage = uncroped_image(roi).clone();
        return croppedImage;
    }

    FUNCTION_ATTRIBUTE
    bool invalid_image_or_not(const cv::Mat &image)
    {
        int blue_color_count = 0;
        int width = 640;
        int height = 480;
        cv::Scalar lower_bound = {114, 46, 25};
        cv::Scalar upper_bound = {126, 255, 180};
        cv::Mat hsv_image; // Load your HSV image here
        // cv ::Mat cropped_image = get_crop_image(image, width, height);
        cv::cvtColor(image, hsv_image, cv::COLOR_BGR2HSV);
        cv::Mat mask_image;
        cv::inRange(hsv_image, lower_bound, upper_bound, mask_image);

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(mask_image, mask_image, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(mask_image, mask_image, cv::MORPH_CLOSE, kernel);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        for (const auto &contour : contours)
        {
            double perimeter = cv::arcLength(contour, true);
            std::vector<cv::Point> approx;
            cv::approxPolyDP(contour, approx, 0.04 * perimeter, true);
            cv::Rect boundingBox = cv::boundingRect(approx);
            int height = boundingBox.height;
            // Extract the width from the bounding rectangle
            int width = boundingBox.width;
            if (width > 40 && height <= 65)
            {
                blue_color_count++;
            }

            // cv::drawContours(image, std::vector<std::vector<cv::Point>>(1, contour), 0, cv::Scalar(0, 0, 255), 2);
        }
        bool status = false;
        if (blue_color_count > 0)
        {
            status = true;
        }
        return status;
        // Calculate the bounding rectangle for the approximated contour
    }

    FUNCTION_ATTRIBUTE
    double calculate_width(const std::vector<cv::Point> &approx)
    {
        cv::Rect boundingBox = cv::boundingRect(approx);

        // Extract the width from the bounding rectangle
        int width = boundingBox.width;
        return width;
    }

    // neoKit algo
    FUNCTION_ATTRIBUTE
    const char *neoKitAlgo(char *bytes, int width, int height)
    {
        long long start = get_now();
        int crop_image_width = 250;
        int crop_image_heigth = 450;
        map<std::string, std::string> colors_output;
        try
        {
            cv::Mat image = cv::Mat(height, width, CV_8UC3, bytes);
            // cv::Mat image;
            // cv::rotate(unrotatedImage, image, cv::ROTATE_90_CLOCKWISE);
            //  bool image_status = invalid_image_or_not(rotatedImage);
            if (true)
            {
                // cv::Mat image = get_crop_image(rotatedImage, crop_image_width, crop_image_heigth);

                // cv::Mat img(height, width, CV_8UC3, Scalar(100, 250, 30));
                //  vector<unsigned char> buffer(bytes, bytes + width * height * 3); // Assuming 3 channels for RGB
                //  cv::Mat image = imdecode(buffer, IMREAD_COLOR);
                cv::Mat hsv_image;
                cvtColor(image, hsv_image, COLOR_BGR2HSV);
                vector<std::string> color_list = {"blue", "light_blue", "yellow", "purple", "green"};

                map<std::string, pair<Scalar, Scalar>> color_and_range = {
                    /*{"light_green", {Scalar(60, 50, 50), Scalar(90, 255, 255)}},
                    //{"green", {Scalar(25, 50, 50), Scalar(70, 255, 255)}},
                    {"pink", {cv::Scalar(129, 50, 70), cv::Scalar(158, 255, 255)}},
                    {"purple", {Scalar(140, 50, 50), Scalar(180, 255, 255)}}};*/
                    /*{"yellow", {cv::Scalar(10, 97, 165), cv::Scalar(18, 255, 255)}},
                    {"dark_blue", {cv::Scalar(97, 0, 74), cv::Scalar(174, 255, 255)}},
                    {"green", {cv::Scalar(22, 43, 66), cv::Scalar(68, 244, 234)}}};*/
                    {"blue", {cv::Scalar(105, 29, 0), cv::Scalar(144, 176, 143)}},
                    {"light_blue", {cv::Scalar(60, 0, 62), cv::Scalar(96, 121, 148)}},
                    {"yellow", {cv::Scalar(8, 85, 121), cv::Scalar(35, 255, 200)}},
                    {"purple", {cv::Scalar(160, 48, 0), cv::Scalar(179, 135, 183)}},
                    {"green", {cv::Scalar(28, 42, 0), cv::Scalar(69, 255, 255)}}};

                for (const std::string &color : color_list)
                {
                    cv::Scalar lower_bound = color_and_range[color].first;
                    cv::Scalar upper_bound = color_and_range[color].second;

                    cv::Mat green_mask;
                    cv::inRange(hsv_image, lower_bound, upper_bound, green_mask);

                    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
                    cv::morphologyEx(green_mask, green_mask, cv::MORPH_OPEN, kernel);
                    cv::morphologyEx(green_mask, green_mask, cv::MORPH_CLOSE, kernel);

                    std::vector<std::vector<cv::Point>> contours;
                    cv::findContours(green_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                    for (const auto &contour : contours)
                    {
                        double perimeter = cv::arcLength(contour, true);
                        std::vector<cv::Point> approx;
                        cv::approxPolyDP(contour, approx, 0.04 * perimeter, true);

                        if (approx.size() == 4 &&
                            calculate_length(approx) > 100 &&
                            calculate_length(approx) < 300 && calculate_width(approx) < 60)
                        {
                            cv::Rect bounding_box = cv::boundingRect(approx);
                            int maxHeight = image.rows;
                            int full_roi_width = bounding_box.width;
                            int medians = bounding_box.width / 4;
                            int roi_width = bounding_box.width - bounding_box.width / 2;
                            int roi_height = bounding_box.height;
                            int roi_x = bounding_box.x + medians;
                            int roi_y = bounding_box.y + roi_height + 2;
                            int roi_bottom = roi_y + roi_height + 2;
                            if (roi_bottom <= maxHeight)
                            {
                                cv::Mat roi = image(cv::Rect(roi_x, roi_y, roi_width, roi_height / 2)).clone();
                                std::pair<cv::Mat, int> result = inner_line_detection(roi);
                                cv::Mat inner_roi = result.first;
                                int line_counts = result.second;
                                image = insertImage(inner_roi, roi_y, roi_x, roi_width, roi_height / 2, image);
                                std::vector<cv::Point> roi_contour = {
                                    cv::Point(roi_x, roi_y),
                                    cv::Point(roi_x + roi_width, roi_y),
                                    cv::Point(roi_x + roi_width, roi_y + roi_height / 2),
                                    cv::Point(roi_x, roi_y + roi_height / 2)};
                                // Extract the ROI from the original image
                                std::vector<std::vector<cv::Point>> roi_contours = {roi_contour};
                                cv::drawContours(image, roi_contours, -1, cv::Scalar(0, 255, 0), 2);
                                if (line_counts == 0 && colors_output.find(color) == colors_output.end())
                                {
                                    colors_output[color] = "Negative";
                                }
                                else if (line_counts == 1)
                                {
                                    colors_output[color] = "Negative";
                                }
                                else if (line_counts == -3)
                                {
                                    colors_output[color] = "Negative";
                                }
                                else if (line_counts >= 2)
                                {
                                    colors_output[color] = "Negative";
                                }
                                std::vector<std::vector<cv::Point>> contour_vec;
                                contour_vec.push_back(approx);
                                cv::drawContours(image, contour_vec, -1, cv::Scalar(0, 255, 0), 2);
                                break;
                            }
                        }
                    }
                }
            }
            json jsonObject;
            json nestedObject;

            for (const auto &entry : colors_output)
            {
                nestedObject[entry.first] = entry.second;
            }

            jsonObject["color_data"] = nestedObject;
            std::string jsonStr = jsonObject.dump();
            char *jsonCharArray = new char[jsonStr.length() + 1];
            std::strcpy(jsonCharArray, jsonStr.c_str());
            delete[] bytes;
            int evalInMillis = static_cast<int>(get_now() - start);
            platform_log("Decode done in %dms\n", evalInMillis);

            return jsonCharArray;
        }
        catch (...)
        {
            json nestedObject;
            json jsonObject;
            colors_output["color"] = "invalid";
            for (const auto &entry : colors_output)
            {
                nestedObject[entry.first] = entry.second;
            }
            jsonObject["color_data"] = nestedObject;
            std::string jsonStr = jsonObject.dump();
            char *jsonCharArray = new char[jsonStr.length() + 1];

            /*long length = image.total() * image.elemSize();
            uint8_t *result = new uint8_t[length];
            // uchar *result = img.data;
            memcpy(result, image.data, length);*/
            std::strcpy(jsonCharArray, jsonStr.c_str());
            // uint8_t greeting[] = "Hello";
            delete[] bytes;
            return jsonCharArray;
        }
    }

    // tamperlokskit
    FUNCTION_ATTRIBUTE
    const char *tamperLoksKit(char *bytes, int width, int height)
    {
        long long start = get_now();
        int crop_image_width = 250;
        int crop_image_heigth = 450;
        map<std::string, std::string> colors_output;
        try
        {

            cv::Mat unrotatedImage = cv::Mat(height, width, CV_8UC3, bytes);
            cv::Mat rotatedImage;
            cv::rotate(unrotatedImage, rotatedImage, cv::ROTATE_90_CLOCKWISE);
            bool image_status = invalid_image_or_not(rotatedImage);
            if (image_status)
            {
                cv::Mat image = get_crop_image(rotatedImage, crop_image_width, crop_image_heigth);

                // cv::Mat img(height, width, CV_8UC3, Scalar(100, 250, 30));
                //  vector<unsigned char> buffer(bytes, bytes + width * height * 3); // Assuming 3 channels for RGB
                //  cv::Mat image = imdecode(buffer, IMREAD_COLOR);
                cv::Mat hsv_image;
                cvtColor(image, hsv_image, COLOR_BGR2HSV);
                vector<std::string> color_list = {"purple", "lime_green", "pink", "yellow", "brown", "dark_green"};

                map<std::string, pair<Scalar, Scalar>> color_and_range = {
                    /*{"light_green", {Scalar(60, 50, 50), Scalar(90, 255, 255)}},
                    //{"green", {Scalar(25, 50, 50), Scalar(70, 255, 255)}},
                    {"pink", {cv::Scalar(129, 50, 70), cv::Scalar(158, 255, 255)}},
                    {"purple", {Scalar(140, 50, 50), Scalar(180, 255, 255)}}};*/
                    {"lime_green", {cv::Scalar(29, 78, 55), cv::Scalar(48, 255, 136)}},
                    {"yellow", {cv::Scalar(17, 86, 131), cv::Scalar(104, 255, 255)}},
                    {"brown", {cv::Scalar(6, 27, 45), cv::Scalar(20, 165, 167)}},
                    {"pink", {cv::Scalar(121, 40, 153), cv::Scalar(180, 203, 255)}},
                    {"purple", {cv::Scalar(107, 21, 26), cv::Scalar(170, 156, 176)}},
                    {"dark_green", {cv::Scalar(57, 28, 87), cv::Scalar(95, 255, 255)}}};

                for (const std::string &color : color_list)
                {
                    cv::Scalar lower_bound = color_and_range[color].first;
                    cv::Scalar upper_bound = color_and_range[color].second;

                    cv::Mat green_mask;
                    cv::inRange(hsv_image, lower_bound, upper_bound, green_mask);

                    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
                    cv::morphologyEx(green_mask, green_mask, cv::MORPH_OPEN, kernel);
                    cv::morphologyEx(green_mask, green_mask, cv::MORPH_CLOSE, kernel);

                    std::vector<std::vector<cv::Point>> contours;
                    cv::findContours(green_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                    for (const auto &contour : contours)
                    {
                        double perimeter = cv::arcLength(contour, true);
                        std::vector<cv::Point> approx;
                        cv::approxPolyDP(contour, approx, 0.04 * perimeter, true);

                        if (approx.size() == 4 &&
                            calculate_length(approx) > 70 &&
                            calculate_length(approx) < 180)
                        {
                            cv::Rect bounding_box = cv::boundingRect(approx);
                            int maxHeight = image.rows;
                            int full_roi_width = bounding_box.width;
                            int medians = bounding_box.width / 4;
                            int roi_width = bounding_box.width - bounding_box.width / 2;
                            int roi_height = bounding_box.height;
                            int roi_x = bounding_box.x + medians;
                            int roi_y = bounding_box.y + roi_height + 2;
                            int roi_bottom = roi_y + roi_height + 2;
                            if (roi_bottom <= maxHeight)
                            {
                                cv::Mat roi = image(cv::Rect(roi_x, roi_y, roi_width, roi_height)).clone();
                                std::pair<cv::Mat, int> result = inner_line_detection(roi);
                                cv::Mat inner_roi = result.first;
                                int line_counts = result.second;
                                image = insertImage(inner_roi, roi_y, roi_x, roi_width, roi_height, image);
                                std::vector<cv::Point> roi_contour = {
                                    cv::Point(roi_x, roi_y),
                                    cv::Point(roi_x + roi_width, roi_y),
                                    cv::Point(roi_x + roi_width, roi_y + roi_height),
                                    cv::Point(roi_x, roi_y + roi_height)};
                                // Extract the ROI from the original image
                                std::vector<std::vector<cv::Point>> roi_contours = {roi_contour};
                                cv::drawContours(image, roi_contours, -1, cv::Scalar(0, 255, 0), 2);
                                if (line_counts == 0 && colors_output.find(color) == colors_output.end())
                                {
                                    colors_output[color] = "Negative";
                                }
                                else if (line_counts == 1)
                                {
                                    colors_output[color] = "Negative";
                                }
                                else if (line_counts == -3)
                                {
                                    colors_output[color] = "Negative";
                                }
                                else if (line_counts >= 2)
                                {
                                    colors_output[color] = "Negative";
                                }
                                std::vector<std::vector<cv::Point>> contour_vec;
                                contour_vec.push_back(approx);
                                cv::drawContours(image, contour_vec, -1, cv::Scalar(0, 255, 0), 2);
                                break;
                            }

                            // cv::rectangle(image, bounding_box, cv::Scalar(0, 255, 0), 2);
                        }
                    }
                }
            }
            // struct_color *color_data = getColorsOutput(colors_output);
            // long length = image.total() * image.elemSize();
            // uint8_t *result = new uint8_t[length];
            // uchar *result = img.data;
            // memcpy(result, image.data, length);
            // uint8_t *imageDataPtr = image.data;
            // size_t imageDataSize = image.total() * image.elemSize();
            json jsonObject;
            json nestedObject;

            for (const auto &entry : colors_output)
            {
                nestedObject[entry.first] = entry.second;
            }

            /*json dataArray;
            for (size_t i = 0; i < imageDataSize; ++i)
            {
                dataArray.push_back(imageDataPtr[i]);
            }*/
            // jsonObject["result"] = "test";
            jsonObject["color_data"] = nestedObject;
            std::string jsonStr = jsonObject.dump();
            char *jsonCharArray = new char[jsonStr.length() + 1];
            std::strcpy(jsonCharArray, jsonStr.c_str());
            // uint8_t greeting[] = "Hello\0";

            // Assign your 'result' image data
            delete[] bytes;
            int evalInMillis = static_cast<int>(get_now() - start);
            platform_log("Decode done in %dms\n", evalInMillis);

            return jsonCharArray;
        }

        catch (...)
        {
            json nestedObject;
            json jsonObject;
            colors_output["color"] = "invalid";
            for (const auto &entry : colors_output)
            {
                nestedObject[entry.first] = entry.second;
            }
            jsonObject["color_data"] = nestedObject;
            std::string jsonStr = jsonObject.dump();
            char *jsonCharArray = new char[jsonStr.length() + 1];

            /*long length = image.total() * image.elemSize();
            uint8_t *result = new uint8_t[length];
            // uchar *result = img.data;
            memcpy(result, image.data, length);*/
            std::strcpy(jsonCharArray, jsonStr.c_str());
            // uint8_t greeting[] = "Hello";
            delete[] bytes;
            return jsonCharArray;
        }
    }

    FUNCTION_ATTRIBUTE
    const char *opencvProcessStream(char *bytes, int width, int height, char *kit_type)
    {
        const char *response;
        if (strcmp(kit_type, "neoKit") == 0)
        {
            response = neoKitAlgo(bytes, width, height);
        }
        if (strcmp(kit_type, "tamperLoks") == 0)
        {
            response = tamperLoksKit(bytes, width, height);
        }
        return response;
    }

    FUNCTION_ATTRIBUTE
    void opencvProcessImage(char *input, char *output)
    {
        long long start = get_now();

        Mat src = imread(input, IMREAD_COLOR);
        // Mat dst;

        // Mat img = src;

        // Mat gray;
        // cvtColor(img, gray, COLOR_BGR2GRAY);
        // Mat mask;
        // threshold(gray, mask, 0, 255, THRESH_BINARY_INV + THRESH_OTSU);
        // Mat white(img.rows, img.cols, CV_8UC3, Scalar(255, 255, 255));
        // Mat result;
        // white.copyTo(result, mask);

        // bitwise_not(result, dst);

        // Scalar lower(10, 10, 10);
        // Scalar upper(256, 256, 256);

        // Mat thresh;
        // inRange(img, lower, upper, thresh);

        // // apply morphology and make 3 channels as mask
        // Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
        // Mat mask1;
        // morphologyEx(thresh, mask1, MORPH_CLOSE, kernel);
        // morphologyEx(mask1, mask1, MORPH_OPEN, kernel);
        // Mat mask3;
        // merge(std::vector<Mat>{mask1, mask1, mask1}, mask3);

        // // blend img with gray using mask
        // Mat result2;
        // bitwise_and(img, mask3, result2);
        // addWeighted(result2, 1, dst, 0.5, 0, result2);

        // dst = result2;
        Mat image = src;
        cv::Mat hsv_image;

        // Mat gray;
        // cvtColor(img, gray, COLOR_BGR2GRAY);
        cvtColor(image, hsv_image, COLOR_BGR2HSV);
        vector<std::string> color_list = {"blue", "light_blue", "Pink", "yellow", "Purple", "green", "orange"};

        map<std::string, pair<Scalar, Scalar>> color_and_range = {
            /*{"light_green", {Scalar(60, 50, 50), Scalar(90, 255, 255)}},
            //{"green", {Scalar(25, 50, 50), Scalar(70, 255, 255)}},
            {"pink", {cv::Scalar(129, 50, 70), cv::Scalar(158, 255, 255)}},
            {"purple", {Scalar(140, 50, 50), Scalar(180, 255, 255)}}};*/
            // {"lime_green", {cv::Scalar(29, 78, 55), cv::Scalar(48, 255, 136)}},
            // {"yellow", {cv::Scalar(17, 86, 131), cv::Scalar(104, 255, 255)}},
            // {"brown", {cv::Scalar(6, 27, 45), cv::Scalar(20, 165, 167)}},
            // {"pink", {cv::Scalar(121, 40, 153), cv::Scalar(180, 203, 255)}},
            // {"purple", {cv::Scalar(107, 21, 26), cv::Scalar(170, 156, 176)}},
            // {"dark_green", {cv::Scalar(57, 28, 87), cv::Scalar(95, 255, 255)}}
            {"blue", {cv::Scalar(102, 1, 0), cv::Scalar(166, 255, 255)}},
            {"light_blue", {cv::Scalar(63, 0, 61), cv::Scalar(166, 255, 255)}},
            {"Pink", {cv::Scalar(300, 50, 70), cv::Scalar(350, 100, 100)}},
            {"yellow", {cv::Scalar(11, 82, 95), cv::Scalar(39, 255, 255)}},
            {"Purple", {cv::Scalar(250, 40, 25), cv::Scalar(270, 80, 75)}},
            {"green", {cv::Scalar(28, 33, 0), cv::Scalar(87, 255, 255)}},
            {"orange", {cv::Scalar(13, 55, 55), cv::Scalar(30, 100, 110)}},
        };

        for (const std::string &color : color_list)
        {
            cv::Scalar lower_bound = color_and_range[color].first;
            cv::Scalar upper_bound = color_and_range[color].second;

            cv::Mat green_mask;
            cv::inRange(hsv_image, lower_bound, upper_bound, green_mask);

            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
            cv::morphologyEx(green_mask, green_mask, cv::MORPH_OPEN, kernel);
            cv::morphologyEx(green_mask, green_mask, cv::MORPH_CLOSE, kernel);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(green_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            String colorandShape = "Not indentified";
            for (const auto &contour : contours)
            {
                double perimeter = cv::arcLength(contour, true);
                std::vector<cv::Point> approx;
                // cv::approxPolyDP(contour, 0.04 * perimeter, true, approx);
                cv::approxPolyDP(contour, approx, 0.04 * perimeter, true);
                RotatedRect minRect = minAreaRect(contour);
                Point2f boxPoints[4];
                minRect.points(boxPoints);
                vector<Point> boxPointsInt(4);
                for (int i = 0; i < 4; ++i)
                {
                    boxPointsInt[i] = boxPoints[i];
                }
                Rect boundingBox = boundingRect(boxPointsInt);

                // Extract the coordinates of the bounding box
                int x = boundingBox.x;
                int y = boundingBox.y;
                int w = boundingBox.width;
                int h = boundingBox.height;
                cv::Rect bounding_box = cv::boundingRect(green_mask);
                platform_log("steste %dms\n", calculate_length(approx));

                if (calculate_length(approx) > 100 &&
                    calculate_length(approx) < 200 && approx.size() < 7 && approx.size() > 2)
                {
                    if (approx.size() == 4)
                    {
                        colorandShape = "quadrilateral_" + color;
                    }
                    else if (approx.size() == 3)
                    {
                        colorandShape = "triangle_" + color;
                    }
                    else if (approx.size() == 5)
                    {
                        colorandShape = "pentagon_" + color;
                    }
                    else if (approx.size() == 6)
                    {
                        colorandShape = "Hexagon_" + color;
                    }
                    Point position(x, y);
                    Scalar backgroundColor = Scalar(255, 255, 255); // White background color (you can choose any color)
                    int fontFace = FONT_HERSHEY_SIMPLEX;
                    double fontScale = 0.6;
                    int thickness = 2;

                    // Get the size of the text to determine the rectangle size
                    Size textSize = cv::getTextSize(colorandShape, fontFace, fontScale, thickness, nullptr);
                    Point textPosition = position;

                    // Calculate the rectangle position and size
                    int rectPadding = 5; // Padding around text
                    Point rectTopLeft = Point(textPosition.x - rectPadding, textPosition.y + rectPadding);
                    Point rectBottomRight = Point(textPosition.x + textSize.width + rectPadding, textPosition.y - textSize.height - rectPadding);

                    // Draw filled rectangle with background color
                    cv::rectangle(image, rectTopLeft, rectBottomRight, backgroundColor, -1);

                    // Overlay text on top of the rectangle
                    cv::putText(image, colorandShape, textPosition, fontFace, fontScale, Scalar(0, 0, 0), thickness);
                    cv::drawContours(image, vector<vector<Point>>{approx}, -1, Scalar(0, 255, 0), 2);
                    // cv::putText(image, colorandShape, position, FONT_HERSHEY_SIMPLEX, 0.6,
                    //             Scalar(0, 0, 0),
                    //             2);
                }
            }
        }

        // write the image to a file
        imwrite(output, image);

        int evalInMillis = static_cast<int>(get_now() - start);
        platform_log("Decode done in %dms\n", evalInMillis);
    }
}
