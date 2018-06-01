#include<stdio.h>
#include<stdlib.h>
#include<dlfcn.h>

int x[2] = {1,2};
int y[2] = {3,4};
int z[2];

int main()
{
    void *handle;
    void (*addvec)(int *,int *,int *,int);
    char *error;

    /*动态载入包含addvec()的标准库*/
    handle = dlopen("./libvector.so",RTLD_LAZY);
    if(!handle)
    {
        fprintf(stderr,"%s\n",dlerror());
        exit(1);
    }

    /*获取刚才加载进来的addvec()函数的指针 */
    addvec = dlsym(handle, "addvec");
    if((error == dlerror()) != NULL){
        fprintf(stderr, "%s\n", error);
        exit(1);
    }

    /*现在能像使用其他函数那样使用addvec()了*/
    addvec(x,y,z,2);
    printf("z = [%d %d]\n",z[0],z[1]);

    /*Unload the shared library*/
    if(dlclose(handle) < 0){
        fprintf(stderr,"%s\n",dlerror());
        exit(1);
    }
    return 0;
}