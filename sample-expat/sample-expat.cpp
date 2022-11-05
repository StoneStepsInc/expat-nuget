//
// This example is copied from Expat's examples/elements.c and adapted
// to compile under C++17.
//

/* This is simple demonstration of how to use expat. This program
   reads an XML document from standard input and writes a line with
   the name of each element to standard output indenting child
   elements by one tab stop more than their parent element.
   It must be used with Expat compiled for UTF-8 output.
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2001-2003 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2004-2006 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2005-2007 Steven Solie <steven@solie.ca>
   Copyright (c) 2016-2022 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2017      Rhodri James <rhodri@wildebeest.org.uk>
   Copyright (c) 2019      Zhongyuan Zhou <zhouzhongyuan@huawei.com>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
   "Software"),  to  deal in  the  Software  without restriction,  including
   without  limitation the  rights  to use,  copy,  modify, merge,  publish,
   distribute, sublicense, and/or sell copies of the Software, and to permit
   persons  to whom  the Software  is  furnished to  do so,  subject to  the
   following conditions:

   The above copyright  notice and this permission notice  shall be included
   in all copies or substantial portions of the Software.

   THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
   EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
   NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
   DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
   USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <expat.h>

#include <cstdio>
#include <cstring>
#include <algorithm>

#ifdef XML_LARGE_SIZE
#  define XML_FMT_INT_MOD "ll"
#else
#  define XML_FMT_INT_MOD "l"
#endif

//
// XML parsing context passed in as user data into callbacks.
//
struct xml_parse_ctx_t {
   size_t depth;
   XML_Parser parser;
};

//
// XML_STATIC is added to the project properties by the Expat Nuget
// package to generate function calls against a static library.
//
static void XMLCALL startElement(void* userData, const XML_Char* name, const XML_Char** atts)
{
   size_t *depthPtr = &(static_cast<xml_parse_ctx_t*>(userData))->depth;

   for(size_t i = 0; i < *depthPtr; i++) {
      putchar(' ');
      putchar(' ');
   }

   printf("%s (", name);

   // number of attributes and their values in this element
   size_t attr_val_count = XML_GetSpecifiedAttributeCount(static_cast<xml_parse_ctx_t*>(userData)->parser);

   // walk attributes and values in pairs
   for(size_t i = 0; i < attr_val_count; i += 2) {
      if(i) {
         putchar(',');
         putchar(' ');
      }

      printf("%s: %s", atts[i], atts[i+1]);
   }

   putchar(')');
   putchar('\n');

   *depthPtr += 1;
}

static void XMLCALL endElement(void* userData, const XML_Char* name)
{
   size_t *depthPtr = &(static_cast<xml_parse_ctx_t*>(userData))->depth;

   *depthPtr -= 1;
}

int main(void)
{
   //
   // Use a small buffer size to simulate reading one buffer at a
   // time from some input stream.
   //
   constexpr size_t BUFSIZE = 10;

   //
   // XML_UNICODE_WCHAR_T should not be defined for projects using
   // this Expat Nuget package. Projects will compile, but will fail
   // to process wide-character XML because Expat can be compiled
   // either for narrow characters or for wide characters, but not
   // both.
   // 
   // This sample XML contains a Katakana character in the abc.b
   // attribute value.
   //
   constexpr XML_Char xml[] = u8R"~~(
      <abc a="1" b=")~~" u8"\u30a1" u8R"~~(">
         abc text
         <!-- XML comment -->
         <def e="z" f="3">
            def text
            <xyz c="2" d="y">
               xyz text
            </xyz>
         </def>
         <ghk g="5"/>
      </abc>
   )~~";

   const XML_Char* cp = xml;

   // XML parsing context to pass into callbacks (the default encoding is UTF-8)
   xml_parse_ctx_t parse_ctx = { 0, XML_ParserCreate(nullptr) };

   if (!parse_ctx.parser) {
      fprintf(stderr, "Couldn't allocate memory for parser\n");
      return 1;
   }

   XML_SetUserData(parse_ctx.parser, &parse_ctx);
   XML_SetElementHandler(parse_ctx.parser, startElement, endElement);

   bool done;
   void *buf = nullptr;

   do {
      // allocates memory as needed and returns the unused part of the allocated buffer
      buf = XML_GetBuffer(parse_ctx.parser, BUFSIZE);

      if (!buf) {
         fprintf(stderr, "Couldn't allocate memory for buffer\n");
         XML_ParserFree(parse_ctx.parser);
         return 1;
      }

      // fill up the buffer or copy whatever we have left from the input string
      const size_t len = std::min<size_t>(sizeof(xml) - sizeof(XML_Char) - (cp - xml), BUFSIZE);

      // we are done if it's the last chunk (may be zero)
      done = len < BUFSIZE;

      // this code typically would read XML from some stream
      if(len)
         strncpy(static_cast<XML_Char*>(buf), cp, len);

      cp += len;

      // parse the XML we collected so far (may or may not call our callbacks, depending on ho much is in the buffer)
      if(XML_ParseBuffer(parse_ctx.parser, static_cast<int>(len), done) == XML_STATUS_ERROR) {
         fprintf(stderr,
            "Parse error at line %" XML_FMT_INT_MOD "u:\n%s\n",
            XML_GetCurrentLineNumber(parse_ctx.parser),
            XML_ErrorString(XML_GetErrorCode(parse_ctx.parser)));
         XML_ParserFree(parse_ctx.parser);
         return 1;
      }

   } while(!done);

   XML_ParserFree(parse_ctx.parser);

   return 0;
}
