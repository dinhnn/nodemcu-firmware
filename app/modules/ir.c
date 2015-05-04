// Module for interfacing with GPIO

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

#include "c_types.h"
#include "c_string.h"

#define os_delay_us ets_delay_us
#define IR_ON() platform_gpio_write(PLATFORM_GPIO_HIGH)
#define IR_OFF() platform_gpio_write(PLATFORM_GPIO_LOW)

uint32 t=0;
static void enableIROut(uint32 khz){
	t = 500/khz;		
}
static void mark(uint32 end)
{
    uint32 now;
    do
    {
        now = system_get_time();
        IR_ON();os_delay_us(t);
        IR_OFF();os_delay_us(t);
    } while (now < end);
}
static void space(uint32 end){
    while (system_get_time() < end)
        asm volatile("nop");
}
#define TOPBIT 0x80000000
#define NEC_HDR_MARK	9000
#define NEC_HDR_SPACE	4500
#define NEC_BIT_MARK	560
#define NEC_ONE_SPACE	1690
#define NEC_ZERO_SPACE	560
#define NEC_RPT_SPACE	2250
static void sendNEC(unsigned long data, int nbits)
{
  int i;
  enableIROut(38);
  uint32 end = system_get_time();
  end+=NEC_HDR_MARK;
  mark(end);
  end+=NEC_HDR_SPACE;
  space(end);
  for (i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      end+=NEC_BIT_MARK;
      mark(end);
      end+=NEC_ONE_SPACE;
      space(end);
    } 
    else {
      end+=NEC_BIT_MARK;
      mark(end);
      end+=NEC_ZERO_SPACE;
      space(end);
    }
    data <<= 1;
  }
  end+=NEC_BIT_MARK;
  mark(end);
  //space(0);
}
// Lua: write( pin, level )
static int lir_nec( lua_State* L )
{
  unsigned long data;
  int nbits;
  data = luaL_checklong(L,1);
  nbits = luaL_checkinteger( L, 2 );
  sendNEC(data,nbits);
  return 0;  
}
// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE ir_map[] = 
{
  { LSTRKEY( "nec" ), LFUNCVAL( lir_nec ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_ir( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_IR, ir_map );
  // Add constants
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}
