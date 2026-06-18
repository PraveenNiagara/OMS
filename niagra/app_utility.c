
#include "app_utility.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_api.h"
#include "eat_type.h"
#include "eat_fs_type.h"
#include "simcom_os.h"

 
 
 INT32 app_nvram_read(UINT8 type, void* ptr, UINT16 len)
 {  
 
     char filename[64]={0};
     UINT32 i, file_len,readed;
     int op_ret,ret;
     SCFILE *file_hdl = NULL;
 
     if(len == 0 || ptr == NULL )
         return -1;
     sAPI_UartPrintf("app_nvram_read\n\r");
     sprintf(filename,"c:/%d",type);
     sprintf(buf,"Filename = %s",filename);
	 sAPI_UartPrintf(buf);     
     
	  file_hdl = sAPI_fopen(filename, "ab");
                if(file_hdl == NULL)
                {
                    sAPI_UartPrintf("sAPI_FsFileOpen err");                  
                    return APP_NVRAM_UNKNOW_ERR;   
                } 
	 
                file_len = sAPI_fsize(file_hdl);
                sprintf(buf,"sAPI_fsize file_len: %d,%d",  file_len,len);
				sAPI_UartPrintf(buf);
				
     /* op_ret = app_fs_get_file_size(fs_handle, &file_len);
     if( op_ret < SC_FS_OK ||file_len<1 )
     {
         sAPI_Debug("FS get file len fail!!!!! error=%x !!!",fs_handle);
         app_fs_close(fs_handle);
         return APP_NVRAM_UNKNOW_ERR;
     }*/
     
     if( file_len > len)
     {
         file_len = len;
     }
     
   /*   readed = sAPI_fread((unsigned char *)textBuf, 800,1,file_hdl);//app_fs_read(fs_handle, ptr, file_len, &readed);
     if(SC_FS_OK != op_ret ||file_len !=readed)
     {
         sAPI_Debug("read file Fail!!! error=%d readed=%d", op_ret, readed);
         sAPI_fclose(ret);//app_fs_close(fs_handle);
         return APP_NVRAM_READ_ERR;
     } */
 
  readed = sAPI_fread((unsigned char *)ptr,len, 1, file_hdl);
                if(readed <= 0)
                {
                    sprintf(buf,"sAPI_FsFileRead err,data: %s, len: %d", ptr,  len);
					sAPI_UartPrintf(buf);
					return APP_NVRAM_READ_ERR;              
                    
                }
				else{
					sprintf(buf,"FileRead,data: %s, len: %d", ptr, len);
					sAPI_UartPrintf(buf);
				}			
				
	  ret = sAPI_fclose(file_hdl);
                if(ret != 0)
                {
                    
					sAPI_UartPrintf("sAPI_fclose err");                
                }
				else{
                    file_hdl = NULL;
                }			
				
     return readed;
 
    
 
}
 /********************************************************************
 * Function    :    app_nvram_save
 * Autor       :    maobin
 * Parameters  :    
 *              type - param type
 *              ptr  -  data point
 *              length - data len
 * Returns     :    
 * Description :    
 ********************************************************************/
 INT32 app_nvram_save(UINT8 type, void* ptr, UINT16 length)
 {
     char filename[64]={0};
     int i, writed, op_ret,ret;
     int fs_handle = 0;
	  SCFILE *file_hdl = NULL;
	  
	 sAPI_UartPrintf("WE AT app_nvram_save\n\r");
	 
     if(length == 0 || ptr == NULL )
     {
         sprintf(buf,"param len=%d ptr=%x, type=%d",length,ptr,type);
		 sAPI_UartPrintf(buf);
         return APP_NVRAM_PARAM_ERR;
     }
 
     sprintf(filename,"c:/%d",type);
     sprintf(buf,"Filename = %s",filename);
	 sAPI_UartPrintf(buf);
	 
	 
	  file_hdl = sAPI_fopen(filename, "wb");
                if(file_hdl == NULL)
                {
                     sAPI_UartPrintf("sAPI_fopen err");
					 return APP_NVRAM_UNKNOW_ERR;
                    
                }      
     
       // ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN); //app_fs_seek(fs_handle, 0, FS_SEEK_BEGIN);
	   
               
			   
     writed = sAPI_fwrite(ptr,length,1,file_hdl);//app_fs_write(fs_handle, ptr, length, &writed);
           if(writed != length)
                {
                    sprintf(buf,"sAPI_fwrite err write length: %d\r\n", writed);                   
                    
                }
				
			else
			{				
               sprintf(buf,"\n\rsAPI_fWrite Success, writed len:%d",writed);
			   sAPI_UartPrintf(buf);
			}			   
     // sAPI_fsync(file_hdl);//app_fs_commit(fs_handle);
	 
	 
	   ret = sAPI_fclose(file_hdl);
                if(ret != 0)
                {
                    
					sAPI_UartPrintf("sAPI_fclose err");                
                }
				else{
                    file_hdl = NULL;
                }
      
      return writed;
 }
 
/*  INT32 app_fs_open(const UINT8 * filename, UINT32 Flag)
 {
     int i;
     UINT8 filename_l[128] = {0};
     for(i=0;i<strlen(filename);i++)
     {
          filename_l[i*2] = filename[i];
          filename_l[i*2+1] = 0x00;
     }
 
     return sAPI_fopen((UINT8*)pfile , "rb");//sAPI_fopen ((UINT16*)filename_l, Flag);//(const SCfileNameInfo *pName_info, const char *pMode);
 }
 
 int app_fs_close(INT32 FileHandle)
 {
     return sAPI_fclose(ret);//sAPI_FsFileClose(FileHandle);
 }
 int app_fs_read(INT32 FileHandle, void * DataPtr, UINT32 Length, UINT32* Read)
 {
     return ret = sAPI_fread((unsigned char *)strBuf, 130,1,file_hdl);//sAPI_FsFileRead(FileHandle, DataPtr, Length, Read);
 }
 int app_fs_write(INT32 FileHandle, void * DataPtr, UINT32 Length, UINT32 * Written)
 {
     return ret = sAPI_fwrite((UINT8*)textBuf, strlen((char*)textBuf),1,file_hdl);//sAPI_FsFileWrite(FileHandle, DataPtr, Length, Written);
 }
 int app_fs_seek(INT32 FileHandle, int Offset, int Whence)
 {
     return ret = sAPI_fseek(file_hdl, 0 , FS_SEEK_BEGIN);//sAPI_FsFileSeek(FileHandle, Offset, Whence);
 }
 int app_fs_commit(INT32 FileHandle)
 {
     return eat_fs_Commit(FileHandle);
 }
 int app_fs_abort(INT32 ActionHandle)
 {
     //return eat_fs_Abort(ActionHandle);
     return 0;
 }
 int app_fs_get_file_size(INT32 FileHandle, UINT32 * Size)
 {
     return sAPI_FsFindFileAndGetSize(FileHandle, Size);
 }
 
 int  app_fs_del(const UINT8* filename)
 {
     int i;
     UINT16 filename_l[50] = {0};
     for(i=0;i<strlen(filename);i++)
     {
          filename_l[i*2] = filename[i];
          filename_l[i*2+1] = 0x00;
     }
     return sAPI_FsFileDelete(filename_l);
 }
 int app_fs_create_dir(const UINT8* DirName)
 {
     int i;
     UINT16 filename_l[50] = {0};
     for(i=0;i<strlen(DirName);i++)
     {
          filename_l[i*2] = DirName[i];
          filename_l[i*2+1] = 0x00;
     }
 
     return eat_fs_CreateDir(filename_l);
 }
 */
#define APP_UPDATE_BUFF_SIZE 0x1000

 /*update from file in filesystem*/
/* void app_update(char* filename)
{
    eat_bool ret = EAT_FALSE;
    void* buff_p = NULL;
    unsigned char *addr;
    unsigned int t1,t2, t_erase=0, t_write=0, c_write=0, read_count=0;
    unsigned int app_datalen = APP_UPDATE_BUFF_SIZE ;
    unsigned int filesize, read_len;
    int testFileHandle ;
    eat_fs_error_enum fs_op_ret;
  //  addr =  (unsigned char *)(eat_get_app_base_addr() + (eat_get_app_space()>>1));
 
	addr = (unsigned char *) FOTASPACE;
	eat_trace( "addr %x, add x%x , base addr x%x" , addr, eat_get_app_base_addr(),(eat_get_app_base_addr() + (eat_get_app_space()>>1)));
    testFileHandle = app_fs_open(filename, FS_READ_ONLY);
    if(testFileHandle<SC_FS_OK )
    {
        eat_trace("sAPI_fopen ():Create File Fail,and Return Error is %x ",testFileHandle);
        return ;
    }
    else
    {
        eat_trace("sAPI_fopen ():Create File Success,and FileHandle is %x ",testFileHandle);
    }
    fs_op_ret = (eat_fs_error_enum)app_fs_get_file_size(testFileHandle,&filesize);
    if(SC_FS_OK != fs_op_ret)
    {
        eat_trace("eat_fs_GetFileSize():Get File Size Fail,and Return Error is %d",fs_op_ret);
        sAPI_fclose(ret);
        return;
    }
    else
    {
        eat_trace("eat_fs_GetFileSize():Get File Size Success and File Size id %d",filesize);
    }

    eat_trace("erase flash addr=%x len=%x", addr,  filesize); 
    t1 = eat_get_current_time();
    ret = eat_flash_erase(addr, filesize);
    t_erase = eat_get_duration_ms(t1);
    if(!ret)
    {
        sAPI_fclose(ret);
        eat_trace("Erase flash failed [0x%08x, %dKByte]", addr,  filesize/1024);
        return;
    }
    read_count = filesize/APP_UPDATE_BUFF_SIZE; //only for testing,so don't case the completeness of file
    eat_trace("need to read file %d",read_count);
    if( read_count == 0)
    {
        //only read once
        read_count=1;
        read_len = filesize;
    }else
    {
        read_count++;
        read_len = APP_UPDATE_BUFF_SIZE;
    }
    buff_p = eat_mem_alloc(app_datalen);
    if( buff_p == NULL)
    {
        eat_trace("mem alloc fail!");
        app_fs_close(testFileHandle);
        return ;
    }
    filesize = 0;
    while(read_count--)
    {
        fs_op_ret = (eat_fs_error_enum)app_fs_read(testFileHandle,buff_p, read_len ,&app_datalen);
        if(SC_FS_OK != fs_op_ret )
        {   
            eat_trace("eat_fs_Read():Read File Fail,and Return Error is %d,Readlen is %d",fs_op_ret,app_datalen);
            app_fs_close(testFileHandle);
            eat_mem_free(buff_p);
            return;
        }
        else
        {
            //eat_trace("eat_fs_Read():Read File Success");
        }

        //eat_trace("START: write flash[0x%x, %dKByte]", APP_DATA_STORAGE_BASE, app_datalen/1024);
        t1 = eat_get_current_time();
        ret = eat_flash_write(addr+filesize , buff_p, app_datalen);
        t2 = eat_get_duration_ms(t1);
        filesize += app_datalen;
        t_write += t2; 
        c_write ++;
        eat_trace("write flash time=%d",t2);
        if(!ret)
        {
            eat_trace("Write flash failed [0x%08x, %dKByte]", addr, app_datalen/1024);
            app_fs_close(testFileHandle);
            eat_mem_free(buff_p);
            return;
        }
    }
    app_fs_close(testFileHandle);
    eat_mem_free(buff_p);

    sAPI_Debug("All use %d write[%d, %d]", c_write, t_erase, t_write);
   //eat_sleep(50);
    //eat_update_app((void*)(eat_get_app_base_addr()), addr, filesize , EAT_PIN_NUM, EAT_PIN_NUM, EAT_FALSE);

    sAPI_Debug("Test App Over");

} */

 
void upperstr(char*s, char*d)
{
    UINT16 len = strlen(s);
    UINT16 i;
    for (i = 0; i < len; i++)
    {
        if (isalpha((int)s[i]))
        {
            d[i] = toupper((int)s[i]);
        }
    }
}

INT32 bytepos(const UINT8* pSrc, UINT16 nSrc,
              const char* pSub, UINT16 startPos)
{
    UINT16 sublen  = strlen(pSub);
    INT32 _return = -1;
    UINT16 index;
    INT32  cmp_return;

    if (sublen > nSrc)
    {
        return _return;
    }

    for (index = startPos; index <= (nSrc - sublen); index++)
    {
        cmp_return = memcmp(&pSrc[index], pSub, sublen);

        if (cmp_return == 0)
        {
            _return = index;
            return _return;
        }
    }

    return _return;
}
 

