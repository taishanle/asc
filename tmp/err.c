/*========================================*\
    文件 : err.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/msg.h>

int main(void)
{
	printf("%d\n",EACCES);
	printf("%d\n",EEXIST);
	printf("%d\n",EFAULT);
	printf("%d\n",EFBIG);
	printf("%d\n",EINTR);
	printf("%d\n",EINVAL);
	printf("%d\n",EINVAL);
	printf("%d\n",EINVAL);
	printf("%d\n",EISDIR);
	printf("%d\n",EISDIR);
	printf("%d\n",ELOOP);
	printf("%d\n",EMFILE);
	printf("%d\n",ENAMETOOLONG);
	printf("%d\n",ENFILE);
	printf("%d\n",ENODEV);
	printf("%d\n",ENOENT);
	printf("%d\n",ENOENT);
	printf("%d\n",ENOMEM);
	printf("%d\n",ENOSPC);
	printf("%d\n",ENOTDIR);
	printf("%d\n",ENXIO);
	printf("%d\n",EOPNOTSUPP);
	printf("%d\n",EOVERFLOW);
	printf("%d\n",EPERM);
	printf("%d\n",EROFS);
	printf("%d\n",ETXTBSY);
	printf("%d\n",EWOULDBLOCK);
	printf("%d\n",EBADF);
	printf("%d\n",ENOTDIR);
	printf("---------------\n");

	printf("%d\n",EAGAIN);
	printf("%d\n",EAGAIN);
	printf("%d\n",EBADF);
	printf("%d\n",EFAULT);
	printf("%d\n",EINTR);
	printf("%d\n",EINVAL);
	printf("%d\n",EINVAL);
	printf("%d\n",EIO);
	printf("%d\n",EISDIR);
	printf("---------------\n");

	printf("%d\n",E2BIG);
	printf("%d\n",EACCES);
	printf("%d\n",EAGAIN);
	printf("%d\n",EFAULT);
	printf("%d\n",EIDRM);
	printf("%d\n",EINTR);
	printf("%d\n",EINVAL);
	printf("%d\n",EINVAL);
	printf("%d\n",EINVAL);
	printf("%d\n",ENOMSG);
	printf("%d\n",ENOMSG);
	printf("%d\n",ENOSYS);
	printf("---------------\n");

	printf("%d,%s\n",EAGAIN,strerror(EAGAIN));
	printf("%d,%s\n",EAGAIN,strerror(EAGAIN));
	printf("%d,%s\n",EBADF,strerror(EBADF));
	printf("%d,%s\n",EFAULT,strerror(EFAULT));
	printf("%d,%s\n",EINTR,strerror(EINTR));
	printf("%d,%s\n",EINVAL,strerror(EINVAL));
	printf("%d,%s\n",EINVAL,strerror(EINVAL));
	printf("%d,%s\n",EIO,strerror(EIO));
	printf("%d,%s\n",EISDIR,strerror(EISDIR));
	printf("---------------\n");

	printf("%d,%s\n",EACCES       ,strerror(EACCES));
	printf("%d,%s\n",EACCES       ,strerror(EACCES));
	printf("%d,%s\n",EPERM        ,strerror(EPERM));
	printf("%d,%s\n",EADDRINUSE   ,strerror(EADDRINUSE));
	printf("%d,%s\n",EADDRNOTAVAIL,strerror(EADDRNOTAVAIL));
	printf("%d,%s\n",EAFNOSUPPORT ,strerror(EAFNOSUPPORT));
	printf("%d,%s\n",EAGAIN       ,strerror(EAGAIN));
	printf("%d,%s\n",EALREADY     ,strerror(EALREADY));
	printf("%d,%s\n",EBADF        ,strerror(EBADF));
	printf("%d,%s\n",ECONNREFUSED ,strerror(ECONNREFUSED));
	printf("%d,%s\n",EFAULT       ,strerror(EFAULT));
	printf("%d,%s\n",EINPROGRESS  ,strerror(EINPROGRESS));
	printf("%d,%s\n",EINTR        ,strerror(EINTR));
	printf("%d,%s\n",EISCONN      ,strerror(EISCONN));
	printf("%d,%s\n",ENETUNREACH  ,strerror(ENETUNREACH));
	printf("%d,%s\n",ENOTSOCK     ,strerror(ENOTSOCK));
	printf("%d,%s\n",ETIMEDOUT    ,strerror(ETIMEDOUT));
	printf("---------------\n");



return 0;
}
