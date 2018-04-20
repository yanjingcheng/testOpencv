/*
* typedef.h - Definition of base data types
*
* Copyright (c) 2016 Rockchip Electronics Co. Ltd.
*
*/
#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define LINE_BUF_STRDE	1920

#define NULL		0
//#define MIN(X, Y)	((X) < (Y) ? (X) : (Y))
//#define MAX(X, Y)	((X) > (Y) ? (X) : (Y))

	typedef	unsigned int		__u32;
	typedef	int			__s32;

	typedef unsigned int		u32;
	typedef signed int		s32;
	typedef unsigned short		u16;
	typedef signed short		s16;
	typedef unsigned char		u8;
	typedef signed char		s8;

	typedef unsigned int		U32;
	typedef signed int		S32;
	typedef unsigned short		U16;
	typedef signed short		S16;
	typedef unsigned char		U8;
	typedef signed char		S8;
	typedef int			BOOL;


	/* legacy, will be removed in the future */
	typedef unsigned int		uint32_t;
	typedef signed int		int32_t;
	typedef unsigned short		uint16_t;
	typedef signed short		int16_t;
	typedef unsigned char		uint8_t;
	typedef signed char		int8_t;
	typedef unsigned long long	uint64_t;
	typedef signed long long	int64_t;

	/* define function point, use for call absolute address */
	typedef void(*p_fun)();
	typedef void(*p_fun_cb)(void *);
	typedef int(*p_fun_buf_cb)(void *, void *);
	typedef signed int spin_lock_id;
#ifdef __cplusplus
}
#endif

#endif
