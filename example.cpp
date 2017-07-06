
#define CONCATENATE_4(      a,b,c,d)  CONCATENATE_4_AGAIN(a,b,c,d)
#define CONCATENATE_4_AGAIN(a,b,c,d)  a ## b ## c ## d

    /* Creates a typedef that's legal/illegal depending on EXPRESSION.       *
	 *      * Note that IDENTIFIER_TEXT is limited to "[a-zA-Z0-9_]*".              *
	 *           * (This may be replaced by static_assert() in future revisions of C++.) */
#define STATIC_ASSERT( EXPRESSION, IDENTIFIER_TEXT)                     \
	  typedef char CONCATENATE_4( static_assert____,      IDENTIFIER_TEXT,  \
			                                ____failed_at_line____, __LINE__ )        \
            [ (EXPRESSION) ? 1 : -1 ]
typedef  int32_t  int4;

STATIC_ASSERT( sizeof(int4) == 4, sizeof_int4_equal_4 );

int main(int argc, char **argv) {
  return 0;
}
