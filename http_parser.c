#include <http_parser.h>
#include <stdint.h>
#include <assert.h>

#ifndef NULL
# define NULL ((void*)0)
#endif

#define MAX_FIELD_SIZE (80*1024)

#define MARK(FOR)                                                    \
do {                                                                 \
  parser->FOR##_mark = p;                                            \
  parser->FOR##_size = 0;                                            \
} while (0)

#define CALLBACK(FOR)                                                \
do {                                                                 \
  if (0 != FOR##_callback(parser, p)) return (p - data);             \
} while (0)

#define CALLBACK2(FOR)                                               \
do {                                                                 \
  if (0 != FOR##_callback(parser)) return (p - data);                \
} while (0)


#if 0
do {                                                                 \
  if (parser->FOR##_mark) {                                          \
    parser->FOR##_size += p - parser->FOR##_mark;                    \
    if (parser->FOR##_size > MAX_FIELD_SIZE) {                       \
      return ERROR;                                                  \
    }                                                                \
    if (parser->on_##FOR) {                                          \
      if (0 != parser->on_##FOR(parser,                              \
                                parser->FOR##_mark,                  \
                                p - parser->FOR##_mark))             \
      {                                                              \
        return ERROR;                                                \
      }                                                              \
    }                                                                \
  }                                                                  \
} while(0)
#endif 

#define DEFINE_CALLBACK(FOR) \
static inline int FOR##_callback (http_parser *parser, const char *p) \
{ \
  if (!parser->FOR##_mark) return 0; \
  assert(parser->FOR##_mark); \
  const char *mark = parser->FOR##_mark; \
  parser->FOR##_size += p - mark; \
  if (parser->FOR##_size > MAX_FIELD_SIZE) return -1; \
  int r = 0; \
  if (parser->on_##FOR) r = parser->on_##FOR(parser, mark, p - mark); \
  parser->FOR##_mark = NULL; \
  return r; \
}

DEFINE_CALLBACK(uri)
DEFINE_CALLBACK(path)
DEFINE_CALLBACK(query_string)
DEFINE_CALLBACK(fragment)
DEFINE_CALLBACK(header_field)
DEFINE_CALLBACK(header_value)

static inline int headers_complete_callback (http_parser *parser)
{
  if (parser->on_headers_complete == NULL) return 0;
  return parser->on_headers_complete(parser);
}

static inline int message_begin_callback (http_parser *parser)
{
  if (parser->on_message_begin == NULL) return 0;
  return parser->on_message_begin(parser);
}

static inline int message_complete_callback (http_parser *parser)
{
  if (parser->on_message_complete == NULL) return 0;
  return parser->on_message_complete(parser);
}

#define CONNECTION "connection"
#define CONTENT_LENGTH "content-length"
#define TRANSFER_ENCODING "transfer-encoding"


static const unsigned char lowcase[] =
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0" "0123456789\0\0\0\0\0\0"
  "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
  "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";


static const uint32_t  usual[] = {
    0xffffdbfe, /* 1111 1111 1111 1111  1101 1011 1111 1110 */

                /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
    0x7ffffff6, /* 0111 1111 1111 1111  1111 1111 1111 0110 */

                /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
};

enum state 
  { s_start = 0

  , s_method_G
  , s_method_GE

  , s_method_P
  , s_method_PU
  , s_method_PO
  , s_method_POS

  , s_method_H
  , s_method_HE
  , s_method_HEA

  , s_method_D
  , s_method_DE
  , s_method_DEL
  , s_method_DELE
  , s_method_DELET

  , s_spaces_before_uri

  , s_schema
  , s_schema_slash
  , s_schema_slash_slash
  , s_host
  , s_port

  , s_path
  , s_query_string_start
  , s_query_string
  , s_fragment_start
  , s_fragment

  , s_http_start
  , s_http_H
  , s_http_HT
  , s_http_HTT
  , s_http_HTTP

  , s_first_major_digit
  , s_major_digit
  , s_first_minor_digit
  , s_minor_digit

  , s_req_line_almost_done

  , s_header_field_start
  , s_header_field
  , s_header_value_start
  , s_header_value

  , s_header_almost_done

  , s_headers_almost_done
  , s_headers_done

  , s_body_chunked
  , s_body_identity
  , s_body_identity_eof
  };

enum header_states 
  { h_general = 0
  , h_C
  , h_CO
  , h_CON
  , h_matching_connection
  , h_matching_content_length
  , h_matching_transfer_encoding
  , h_connection
  , h_content_length
  , h_transfer_encoding
  , h_encoding_C
  , h_encoding_CH
  , h_encoding_CHU
  , h_encoding_CHUN
  , h_encoding_CHUNK
  , h_encoding_CHUNKE
  , h_encoding_CHUNKED
  , h_connection_K
  , h_connection_C
  };

enum flags
  { F_CHUNKED = 0x0001
  };

#define ERROR (p - data);
#define CR '\r'
#define LF '\n'
#define LOWER(c) (unsigned char)(c | 0x20)

size_t http_parser_execute (http_parser *parser, const char *data, size_t len)
{
  char c, ch; 
  const char *p, *pe;

  enum state state = parser->state;
  enum header_states header_state = parser->header_state;
  size_t header_index = parser->header_index;

  if (len == 0 && state == s_body_identity_eof) {
    CALLBACK2(message_complete);
    return 0;
  }

  if (parser->header_field_mark)   parser->header_field_mark   = data;
  if (parser->header_value_mark)   parser->header_value_mark   = data;
  if (parser->fragment_mark)       parser->fragment_mark       = data;
  if (parser->query_string_mark)   parser->query_string_mark   = data;
  if (parser->path_mark)           parser->path_mark           = data;
  if (parser->uri_mark)            parser->uri_mark            = data;

  for (p=data, pe=data+len; p != pe; p++) {
    ch = *p;
    switch (state) {
      case s_start:
      {
        parser->flags = 0;
        parser->content_length = -1;

        CALLBACK2(message_begin);

        switch (ch) {
          /* GET */
          case 'G':
            state = s_method_G;
            break;

          /* POST, PUT */
          case 'P':
            state = s_method_P;
            break;

          /* HEAD */
          case 'H':
            state = s_method_H;
            break;

          /* DELETE */
          case 'D':
            state = s_method_D;
            break;

          case CR:
          case LF:
            break;

          default:
            return ERROR;
        }
        break;
      }
       
      /* GET */

      case s_method_G:
        if (ch != 'E') return ERROR;
        state = s_method_GE;
        break;

      case s_method_GE:
        if (ch != 'T') return ERROR;
        parser->method = HTTP_GET;
        state = s_spaces_before_uri;
        break;

      /* HEAD */

      case s_method_H:
        if (ch != 'E') return ERROR;
        state = s_method_HE;
        break;

      case s_method_HE:
        if (ch != 'A') return ERROR;
        state = s_method_HEA;
        break;

      case s_method_HEA:
        if (ch != 'D') return ERROR;
        parser->method = HTTP_HEAD;
        state = s_spaces_before_uri;
        break;

      /* POST, PUT */

      case s_method_P:
        switch (ch) {
          case 'O':
            state = s_method_PO;
            break;

          case 'U':
            state = s_method_PU;
            break;

          default:
            return ERROR;
        }
        break;

      /* PUT */

      case s_method_PU:
        if (ch != 'T') return ERROR;
        parser->method = HTTP_PUT;
        state = s_spaces_before_uri;
        break;

      /* POST */

      case s_method_PO:
        if (ch != 'S') return ERROR;
        state = s_method_POS;
        break;

      case s_method_POS:
        if (ch != 'T') return ERROR;
        parser->method = HTTP_POST;
        state = s_spaces_before_uri;
        break;

      /* DELETE */

      case s_method_D:
        if (ch != 'E') return ERROR;
        state = s_method_DE;
        break;

      case s_method_DE:
        if (ch != 'L') return ERROR;
        state = s_method_DEL;
        break;

      case s_method_DEL:
        if (ch != 'E') return ERROR;
        state = s_method_DELE;
        break;

      case s_method_DELE:
        if (ch != 'T') return ERROR;
        state = s_method_DELET;
        break;

      case s_method_DELET:
        if (ch != 'E') return ERROR;
        parser->method = HTTP_DELETE;
        state = s_spaces_before_uri;
        break;


      case s_spaces_before_uri:
      {
        if (ch == ' ') break;

        if (ch == '/') {
          MARK(uri);
          MARK(path);
          state = s_path;
          break;
        }

        c = LOWER(ch);

        if (c >= 'a' && c <= 'z') {
          MARK(uri);
          state = s_schema;
          break;
        }

        return ERROR;
      }

      case s_schema:
      {
        c = LOWER(ch);

        if (c >= 'a' && c <= 'z') break;

        if (ch == ':') {
          state = s_schema_slash;
          break;
        }

        return ERROR;
      }

      case s_schema_slash:
        if (ch != '/') return ERROR;
        state = s_schema_slash_slash;
        break;

      case s_schema_slash_slash:
        if (ch != '/') return ERROR;
        state = s_host;
        break;

      case s_host:
      {
        c = LOWER(ch);
        if (c >= 'a' && c <= 'z') break;
        if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-') break;
        switch (ch) {
          case ':':
            state = s_port;
            break;
          case '/':
            MARK(path);
            state = s_path;
            break;
          case ' ':
            /* The request line looks like:
             *   "GET http://foo.bar.com HTTP/1.1"
             * That is, there is no path.
             */
            CALLBACK(uri);
            state = s_http_start;
            break;
          default:
            return ERROR;
        }
        break;
      }

      case s_port:
      {
        if (ch >= '0' && ch <= '9') break;
        switch (ch) {
          case '/':
            MARK(path);
            state = s_path;
            break;
          case ' ':
            /* The request line looks like:
             *   "GET http://foo.bar.com:1234 HTTP/1.1"
             * That is, there is no path.
             */
            CALLBACK(uri);
            state = s_http_start;
            break;
          default:
            return ERROR;
        }
        break;
      }

      case s_path:
      {
        if (usual[ch >> 5] & (1 << (ch & 0x1f))) break;

        switch (ch) {
          case ' ':
            CALLBACK(uri);
            CALLBACK(path);
            state = s_http_start;
            break;
          case CR:
            CALLBACK(uri);
            CALLBACK(path);
            parser->http_minor = 9;
            state = s_req_line_almost_done;
            break;
          case LF:
            CALLBACK(uri);
            CALLBACK(path);
            parser->http_minor = 9;
            state = s_header_field_start;
            break;
          case '?':
            CALLBACK(path);
            state = s_query_string_start;
            break;
          case '#':
            CALLBACK(path);
            state = s_fragment_start;
            break;
          default:
            return ERROR;
        }
        break;
      }

      case s_query_string_start:
      {
        if (usual[ch >> 5] & (1 << (ch & 0x1f))) {
          MARK(query_string);
          state = s_query_string;
          break;
        }

        switch (ch) {
          case '?':
            break; // XXX ignore extra '?' ... is this right? 
          case ' ':
            CALLBACK(uri);
            state = s_http_start;
            break;
          case CR:
            CALLBACK(uri);
            parser->http_minor = 9;
            state = s_req_line_almost_done;
            break;
          case LF:
            CALLBACK(uri);
            parser->http_minor = 9;
            state = s_header_field_start;
            break;
          case '#':
            state = s_fragment_start;
            break;
          default:
            return ERROR;
        }
        break;
      }

      case s_query_string:
      {
        if (usual[ch >> 5] & (1 << (ch & 0x1f))) break;

        switch (ch) {
          case ' ':
            CALLBACK(uri);
            CALLBACK(query_string);
            state = s_http_start;
            break;
          case CR:
            CALLBACK(uri);
            CALLBACK(query_string);
            parser->http_minor = 9;
            state = s_req_line_almost_done;
            break;
          case LF:
            CALLBACK(uri);
            CALLBACK(query_string);
            parser->http_minor = 9;
            state = s_header_field_start;
            break;
          case '#':
            CALLBACK(query_string);
            state = s_fragment_start;
            break;
          default:
            return ERROR;
        }
        break;
      }

      case s_fragment_start:
      {
        if (usual[ch >> 5] & (1 << (ch & 0x1f))) {
          MARK(fragment);
          state = s_fragment;
          break;
        }

        switch (ch) {
          case ' ':
            CALLBACK(uri);
            state = s_http_start;
            break;
          case CR:
            CALLBACK(uri);
            parser->http_minor = 9;
            state = s_req_line_almost_done;
            break;
          case LF:
            CALLBACK(uri);
            parser->http_minor = 9;
            state = s_header_field_start;
            break;
          case '?':
            MARK(fragment);
            state = s_fragment;
            break;
          case '#':
            break;
          default:
            return ERROR;
        }
        break;
      }

      case s_fragment:
      {
        if (usual[ch >> 5] & (1 << (ch & 0x1f))) break;

        switch (ch) {
          case ' ':
            CALLBACK(uri);
            CALLBACK(fragment);
            state = s_http_start;
            break;
          case CR:
            CALLBACK(uri);
            CALLBACK(fragment);
            parser->http_minor = 9;
            state = s_req_line_almost_done;
            break;
          case LF:
            CALLBACK(uri);
            CALLBACK(fragment);
            parser->http_minor = 9;
            state = s_header_field_start;
            break;
          case '?':
          case '#':
            break;
          default:
            return ERROR;
        }
        break;
      }

      case s_http_start:
        switch (ch) {
          case 'H':
            state = s_http_H;
            break;
          case ' ':
            break;
          default:
            return ERROR;
        }
        break;

      case s_http_H:
        if (ch != 'T') return ERROR;
        state = s_http_HT;
        break;

      case s_http_HT:
        if (ch != 'T') return ERROR;
        state = s_http_HTT;
        break;

      case s_http_HTT:
        if (ch != 'P') return ERROR;
        state = s_http_HTTP;
        break;

      case s_http_HTTP:
        if (ch != '/') return ERROR;
        state = s_first_major_digit;
        break;

      /* first digit of major HTTP version */
      case s_first_major_digit:
        if (ch < '1' || ch > '9') return ERROR;
        parser->http_major = ch - '0';
        state = s_major_digit;
        break;

      /* major HTTP version or dot */
      case s_major_digit:
      {
        if (ch == '.') {
          state = s_first_minor_digit;
          break;
        }

        if (ch < '0' || ch > '9') return ERROR;

        parser->http_major *= 10;
        parser->http_major += ch - '0';

        if (parser->http_major > 999) return ERROR;
        break;
      }

      /* first digit of minor HTTP version */
      case s_first_minor_digit:
        if (ch < '0' || ch > '9') return ERROR;
        parser->http_minor = ch - '0';
        state = s_minor_digit;
        break;

      /* minor HTTP version or end of request line */
      case s_minor_digit:
      {
        if (ch == CR) {
          state = s_req_line_almost_done;
          break;
        }

        if (ch == LF) {
          state = s_header_field_start;
          break;
        }

        /* XXX allow spaces after digit? */

        if (ch < '0' || ch > '9') return ERROR;

        parser->http_minor *= 10;
        parser->http_minor += ch - '0';

        if (parser->http_minor > 999) return ERROR;
        break;
      }

      /* end of request line */
      case s_req_line_almost_done:
      {
        if (ch != LF) return ERROR;
        state = s_header_field_start;
        break;
      }

      case s_header_field_start:
      {
        if (ch == CR) {
          state = s_headers_almost_done;
          break;
        }

        if (ch == LF) {
          state = s_headers_done;
          break;
        }

        c = LOWER(ch);

        if (c < 'a' || 'z' < c) return ERROR;

        MARK(header_field);

        header_index = 0;
        state = s_header_field;

        switch (c) {
          case 'c':
            header_state = h_C;
            break;

          case 't':
            header_state = h_matching_transfer_encoding;
            break;

          default:
            header_state = h_general;
            break;
        }
        break;
      }

      case s_header_field:
      {
        header_index++;

        c = lowcase[(int)ch];

        if (c) {
          switch (header_state) {
            case h_general:
              break;

            case h_C:
              header_state = (c == 'o' ? h_CO : h_general);
              break;

            case h_CO:
              header_state = (c == 'n' ? h_CON : h_general);
              break;

            case h_CON:
              switch (c) {
                case 'n':
                  header_state = h_matching_connection;
                  break;
                case 't':
                  header_state = h_matching_content_length;
                  break;
                default:
                  header_state = h_general;
                  break;
              }
              break;

            /* connection */

            case h_matching_connection:
              if (header_index > sizeof(CONNECTION)-1
                  || c != CONNECTION[header_index]) {
                header_state = h_general;
              } else if (header_index == sizeof(CONNECTION)-1) {
                header_state = h_connection;
              }
              break;

            /* content-length */

            case h_matching_content_length:
              if (header_index > sizeof(CONTENT_LENGTH)-1
                  || c != CONTENT_LENGTH[header_index]) {
                header_state = h_general;
              } else if (header_index == sizeof(CONTENT_LENGTH)-1) {
                header_state = h_content_length;
              }
              break;

            /* transfer-encoding */

            case h_matching_transfer_encoding:
              if (header_index > sizeof(TRANSFER_ENCODING)-1
                  || c != TRANSFER_ENCODING[header_index]) {
                header_state = h_general;
              } else if (header_index == sizeof(TRANSFER_ENCODING)-1) {
                header_state = h_transfer_encoding;
              }
              break;

            default:
              assert(0 && "Unknown header_state");
              break;
          }
          break;
        }

        if (ch == ':') {
          CALLBACK(header_field);
          state = s_header_value_start;
          break;
        }

        if (ch == CR) {
          state = s_header_almost_done;
          CALLBACK(header_field);
          break;
        }

        if (ch == LF) {
          CALLBACK(header_field);
          state = s_header_field_start;
          break;
        }

        return ERROR;
      }

      case s_header_value_start:
      {
        if (ch == ' ') break;

        if (ch == CR) {
          header_state = h_general;
          state = s_header_almost_done;
          break;
        }

        if (ch == LF) {
          state = s_header_field_start;
          break;
        }

        MARK(header_value);

        c = lowcase[(int)ch];

        if (!c) {
          state = s_header_value;
          header_state = h_general;
        } else {
          switch (header_state) {
            case h_transfer_encoding:
              /* looking for 'Transfer-Encoding: chunked' */
              if ('c' == c) {
                header_state = h_encoding_C;
              } else {
                header_state = h_general;
              }
              break;

            case h_content_length:
              if (ch < '0' || ch > '9') return ERROR;
              parser->content_length = ch - '0';
              break;

            case h_connection:
              /* looking for 'Connection: keep-alive' */
              if (c == 'k') {
                header_state = h_connection_K;
              /* looking for 'Connection: close' */
              } else if (c == 'c') {
                header_state = h_connection_C;
              } else {
                header_state = h_general;
              }
              break;

            default:
              state = s_header_value;
              header_state = h_general;
              break;
          }
          break;
        }
        break;
      }

      case s_header_value:
      {
        c = lowcase[(int)ch];

        if (!c) {
          if (ch == CR) {
            CALLBACK(header_value);
            state = s_header_almost_done;
            break;
          }

          if (ch == LF) {
            CALLBACK(header_value);
            state = s_header_field_start;
            break;
          }
          break;
        }

        switch (header_state) {
          case h_connection:
          case h_transfer_encoding:
            assert(0 && "Shouldn't get here.");
            break;

          case h_general:
            break;

          case h_content_length:
            if (ch < '0' || ch > '9') return ERROR;
            parser->content_length *= 10;
            parser->content_length += ch - '0';
            break;

          /* Transfer-Encoding: chunked */

          case h_encoding_C:
            header_state = (c == 'h' ? h_encoding_CH : h_general);
            break;
          case h_encoding_CH:
            header_state = (c == 'u' ? h_encoding_CHU : h_general);
            break;
          case h_encoding_CHU:
            header_state = (c == 'n' ? h_encoding_CHUN : h_general);
            break;
          case h_encoding_CHUN:
            header_state = (c == 'k' ? h_encoding_CHUNK : h_general);
            break;
          case h_encoding_CHUNK:
            header_state = (c == 'e' ? h_encoding_CHUNKE : h_general);
            break;
          case h_encoding_CHUNKE:
            if (c == 'd') {
              parser->flags |= F_CHUNKED;
              header_state = h_encoding_CHUNKED;
            }
            break;
          case h_encoding_CHUNKED:
            if (ch != ' ') return ERROR;
            break;

          /* looking for 'Connection: keep-alive' */
          /* looking for 'Connection: close' */
          case h_connection_K: 
          case h_connection_C: 
            header_state = h_general;
            break;

          default:
            state = s_header_value;
            header_state = h_general;
            break;
        }
        break;
      }

      case s_header_almost_done:
        if (ch != LF) return ERROR;
        state = s_header_field_start;
        break;

      case s_headers_almost_done:
        if (ch != LF) return ERROR;
        CALLBACK2(headers_complete);
        
        if (parser->flags & F_CHUNKED) {
          state = s_body_chunked;
        } else {
          if (parser->content_length == 0) {
            CALLBACK2(message_complete);
            state = s_start;
          } else if (parser->content_length < 0) {
            state = s_body_identity_eof;
          } else {
            state = s_body_identity;
          }
        }
        break;

      /* read until EOF */
      case s_body_identity_eof:
        break;

      case s_body_identity:
        break;

      case s_body_chunked:
        break;

      default:
        assert(0 && "unhandled state");
        return ERROR;
    }
  }

  CALLBACK(header_field);
  CALLBACK(header_value);
  CALLBACK(fragment);
  CALLBACK(query_string);
  CALLBACK(path);
  CALLBACK(uri);

  parser->state = state;
  parser->header_state = header_state;
  parser->header_index = header_index;

  return len;
}

void
http_parser_init (http_parser *parser, enum http_parser_type type)
{
  if (type == HTTP_REQUEST) {
    parser->state = s_start;
  } else {
    assert(0 && "responses not supported yet");
  }

  parser->on_message_begin = NULL;
  parser->on_path = NULL;
  parser->on_query_string = NULL;
  parser->on_uri = NULL;
  parser->on_fragment = NULL;
  parser->on_header_field = NULL;
  parser->on_header_value = NULL;
  parser->on_headers_complete = NULL;
  parser->on_body = NULL;
  parser->on_message_complete = NULL;
}
