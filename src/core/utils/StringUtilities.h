/**
 * @file
 * This file helper functions for manipulating RlStrings in RevBayes.
 *
 * @brief Namespace containing helper functions for RlStrings
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 *
 * $Id$
 */
#ifndef RlStringUtilities_H
#define RlStringUtilities_H

#include <stddef.h>
#include <sstream> // IWYU pragma: keep
#include <vector>

namespace StringUtilities {
    
    
    int                         asIntegerNumber(const std::string& s);                                              //!< Checks if a string is an integer number
    void                        fillWithSpaces(std::string &s, int l, bool left);                                   //!< Fill up the string with spaces either left aligned or right aligned
    size_t                      findFirstOf(const std::string &s, char c);
    size_t                      findFirstOf(const std::string &a, const std::string &b);
    size_t                      findLastOf(const std::string &s, char c);
    std::string&                firstCharToUpper(std::string& str);                                                 //!< Convert string's first character to upper case
    void                        formatFixedWidth(std::string &s, int l, bool left);                                 //!< Format the string so that it fits the width (either truncating or adding spaces)
    std::string                 formatStringForScreen(  const std::string  &s,
                                                        const std::string  &firstLinePad,
                                                        const std::string  &hangingPad,
                                                        size_t              screenWidth);                           //!< Format string for output to screen
    std::string                 formatTabWrap(std::string s, size_t tabs, size_t width, bool removeFormat=true);    //!< Wraps texts.
    std::string                 getFileContentsAsString(const std::string& s);                                      //!< Convert the file contents to a string
    std::string                 getStringWithDeletedLastPathComponent(const std::string& s);                        //!< Convert the file contents to a string
    std::string                 getLastPathComponent(const std::string& s);                                         //!< Find the last component of a file path
    bool                        isFormattingChar(char c) ;
    bool                        isIntegerNumber(const std::string& s);                                              //!< Checks if a string is an integer number
    bool                        isNumber(const std::string& s);                                                     //!< Checks if a string is a number
    std::string                 oneLiner(const std::string& input, size_t maxLen);                                  //!< Get a one-liner of specified length
    void                        replaceSubstring(std::string& str, const std::string& oldStr, const std::string& newStr);
    void                        replaceAllOccurrences(std::string& str, char old_ch, char new_ch);
    void                        stringSplit(std::string str, std::string delim, std::vector<std::string>& results, bool trim = false); //!< Split a string into pieces
    void                        toLower(std::string& str);                                                          //!< Convert string's characters to lower case
    std::string                 toString(double x, int digits=6);                                                   //!< Convert string's characters to lower case
    
    /**
     * Generic to_string function
     * @param value The value that we transform into a string
     * @return The input value as a string.
     */
    template <typename T>
    std::string to_string(T value) {
        std::ostringstream os;
        os << value;
        return os.str();
    }
    
}

// TODO: Is there a better way for these global operators than duplicating the code for each namespace? (Sebastian)
namespace RevLanguage {

    std::string                     operator+(const std::string& A, double B);
    std::string                     operator+(const std::string& A, int B);
    std::string                     operator+(const std::string& A, long B);
    std::string                     operator+(const std::string& A, size_t B);
    std::string                     operator+(double A, const std::string& B);
    std::string                     operator+(int A, const std::string& B);
    std::string                     operator+(long A, const std::string& B);
    std::string                     operator+(size_t A, const std::string& B);

}

namespace RevBayesCore {
    
    template <class valueType>
    class RbVector;
    
    std::string                     operator+(const std::string& A, double B);
    std::string                     operator+(const std::string& A, int B);
    std::string                     operator+(const std::string& A, long B);
    std::string                     operator+(const std::string& A, size_t B);
    std::string                     operator+(const std::string& A, const RbVector<double> &B);
    std::string                     operator+(const std::string& A, const RbVector<long> &B);
    std::string                     operator+(double A, const std::string& B);
    std::string                     operator+(int A, const std::string& B);
    std::string                     operator+(long A, const std::string& B);
    std::string                     operator+(size_t A, const std::string& B);
    
}


std::string                     operator+(const std::string& A, double B);
std::string                     operator+(const std::string& A, int B);
std::string                     operator+(const std::string& A, long B);
std::string                     operator+(const std::string& A, size_t B);
//std::string                     operator+(const std::string& A, const RevBayesCore::RbVector<double> &B);
//std::string                     operator+(const std::string& A, const RevBayesCore::RbVector<long> &B);
std::string                     operator+(double A, const std::string& B);
std::string                     operator+(int A, const std::string& B);
std::string                     operator+(long A, const std::string& B);
std::string                     operator+(size_t A, const std::string& B);


#endif
