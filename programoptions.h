#ifndef PROGRAMOPTIONS_H
#define PROGRAMOPTIONS_H

#include "codepage.h"

#include <string>
#include <utility>
#include <vector>
#include <iosfwd>
#include <sstream>

#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>

class ProgramOptions {
public:

	using CodePoints = std::vector< CodePointRange >;
	using CodePointssCIt = CodePoints::const_iterator;

	enum class FileFormat {
		BMP, JPEG, PNG, DDS
	};

	enum class LogLevel {
		NONE, LOW, MEDIUM, HIGH
	};

	void extractExtension()
	{
		auto pointPos = mImageFilename.find_last_of('.');

		if (pointPos == std::string::npos )
			throw std::invalid_argument{"image filename without extension"};
		if (pointPos + 1 >= mImageFilename.length())
			throw std::invalid_argument{"image filename without extension"};

		mImageExtension = mImageFilename.substr(pointPos+1);
	}

	ProgramOptions(int argc, char* argv[])
	{
		namespace po = boost::program_options;

		desc.add_options()
				("help,h", "produce help message")

				("input-ttf,i", po::value<std::string>(&mInputFont),
				 "input ttf filename")

				("font,f", po::value<std::string>(&mFontName),
				 "name of font to use")

				("image-filename", po::value<std::string>(&mImageFilename)
				 ->default_value("image.bmp"),
				 "output image filename")

				("size,s", po::value<int>(&mSize)->default_value(32),
				 "size of font in pixels")

				("resolution,r", po::value<int>(&mResolution)->default_value(96),
				 "resolution in dpi")

				("output,o", po::value<std::string>(&mOutputFontDef)->default_value("out.fontdef"),
				 "outupt fontdef file")

				("append-mode,a",
				 "use if you want to append to an existing fontdef")

				("verbose,v", po::value<int>(&mVerboseLevel)->default_value(false))

				("codepage,c", po::value< std::vector<CodePointRange>>(&mCodePages)
				 ->multitoken()
				 ->default_value({{33,166}}, "33-166"),"range of cod pages")
				;

		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help"))
			mMustPrintHelp = true;

		if (vm.count("image-filename"))
		{
			if (verboseLevel() == LogLevel::MEDIUM)
				std::cout << "image-filename not set default is: " << output() << std::endl;
		}

		bool must_throw = false;
		std::ostringstream os;

		if (!exist("input-ttf"))
		{
			os << "missing filename arguments!\n";
			must_throw = true;
		}

		if (!exist("font"))
		{
			os << "missing font arguments!\n";
			must_throw = true;
		}

		mIsAppend = exist("append-mode");

		if (must_throw)
		{
			os << *this << std::endl;
			throw std::invalid_argument{os.str()};
		}

		if (mustPrintHelp())
			std::cout << desc << std::endl;

		extractExtension();

		logParameters();
	}

	void logParameters()
	{
		if (verboseLevel() == LogLevel::HIGH)
		{
			std::cout
			   << "\nfont filename:    " << inputFont()
			   << "\nfontname:       " << fontName()
			   << "\noutput image:   " << imageFilename()
			   << "\nextension:      " << imageExtension()
			   << "\nresolution:     " << resolution()
			   << "\nsize:           " << size()
			   << "\noutput fontdef: " << output()
			   << "\nappend mode:    " << std::boolalpha << isAppend()
			   << "\ncode pages:     ";
			for(const auto& cp : *this)
				std::cout << cp << ' ';
			std::cout
			   << "\nverbose level:  " << static_cast<int>(verboseLevel())
			   << std::endl;
		}
	}

	bool mustPrintHelp() const noexcept
	{
		return mMustPrintHelp;
	}

	friend std::ostream& operator << (std::ostream& os, const ProgramOptions& po)
	{
		return os << po.desc;
	}

	const std::string& fontName() const noexcept
	{
		return mFontName;
	}

	const std::string& output() const noexcept
	{
		return mOutputFontDef;
	}

	const std::string& inputFont() const noexcept
	{
		return mInputFont;
	}

	int size() const noexcept
	{
		return mSize;
	}

	int resolution() const noexcept
	{
		return mResolution;
	}

	const std::string& imageFilename() const noexcept
	{
		return mImageFilename;
	}

	const std::string& imageExtension() const noexcept
	{
		return mImageExtension;
	}

	bool isAppend() const noexcept
	{
		return mIsAppend;
	}

	LogLevel verboseLevel() const noexcept
	{
		return static_cast<LogLevel>(mVerboseLevel);
	}

	CodePointssCIt begin() const noexcept
	{
		return std::begin(mCodePages);
	}

	CodePointssCIt end() const noexcept
	{
		return std::end(mCodePages);
	}

private:
	bool exist(const std::string& str) const noexcept
	{
		return vm.find(str) != std::end(vm);
	}

private:
	boost::program_options::options_description desc{"Options"};
	boost::program_options::variables_map vm;

	std::string mInputFont;
	std::string mOutputFontDef;
	std::string mFontName;
	std::string mImageFilename;
	std::string mImageExtension;
	std::vector<CodePointRange> mCodePages;
	int mSize;
	int mResolution;

	int mVerboseLevel = 0;
	bool mMustPrintHelp = false;
	bool mIsAppend = false;

	static const int HIGH_VERBOSE = 3;
	static const int MEDIUM_VERBOSE = 2;
	static const int LOW_VERBOSE = 1;
	static const int NO_VERBOSE = 0;
};

#endif // PROGRAMOPTIONS_H
