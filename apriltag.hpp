//
// Created by huangqinjin on 5/31/17.
//

#ifndef APRILTAG_HPP
#define APRILTAG_HPP

extern "C"
{
    struct zarray;
    struct apriltag_family;
    struct apriltag_detector;
    struct apriltag_detection;
    struct image_u8;
}

namespace cv
{
    class Mat;
}

namespace apriltag
{
    template<typename T>
    using identity = T;

    class family
    {
    public:
        static family get(const char* name);

        explicit family(apriltag_family* tf = nullptr);
        family(family&&);
        ~family();
        family& operator=(const family&) = delete;

        const char* name() const;

        apriltag_family* impl;
    };

    family tag16h5();
    family tag25h7();
    family tag25h9();
    family tag36h10();
    family tag36h11();
    family tag36artoolkit();

    class detection
    {
    public:
        class tag
        {
        public:
            tag() = delete;
            tag(const tag&) = delete;
            tag& operator=(const tag&) = delete;

            int id() const;
            identity<const double(&)[2]> center() const;
            identity<const double(&)[2]> corner(int i) const;
            identity<const double(&)[4][2]> corners() const;
            operator identity<const double(&)[4][2]>() const;
        };

        class iterator
        {
        public:
            using value_type = const tag;
            using pointer = const tag*;
            using reference	= const tag&;

            iterator& operator++() { ++impl; return *this; }
            iterator operator++(int) { iterator tmp(*this); ++(*this); return tmp; }
            bool operator==(const iterator& other) const { return impl == other.impl; }
            bool operator!=(const iterator& other) const { return !operator==(other); }
            pointer operator->() const { return reinterpret_cast<pointer>(*impl); }
            reference operator*() const { return *operator->(); }

            apriltag_detection** impl;
        };

        using const_iterator = iterator;

        explicit detection(zarray* d = nullptr);
        detection(detection&&);
        ~detection();
        detection(const detection&) = delete;
        detection& operator=(const detection&) = delete;

        bool empty() const;
        int size() const;
        const_iterator begin() const;
        const_iterator end() const;
        const tag& operator[](int i) const;

        void draw(cv::Mat& img, int flags = 0);

        zarray* impl;
    };

    class detector
    {
    public:
        explicit detector(apriltag_detector*);
        explicit detector(family tf = tag36h11());
        detector(detector&&);
        ~detector();
        detector& operator=(const detector&) = delete;
        int threads() const;
        void threads(int n);
        int level() const;
        void level(int n);
        detection detect(image_u8* img);
        detection detect(cv::Mat& img);
        apriltag_detector* impl;
        family tf;
    };
}


#endif //APRILTAG_HPP
