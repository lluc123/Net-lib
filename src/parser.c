#include "parser.h"

const struct http_param l_error_param = {0,0};

int init_param(char * key, char * value, struct http_param* p)
{
    size_t k = strlen(key)+1;
    size_t v = strlen(value)+1;
    p->pname = malloc(k);
    p->pvalue = malloc(v);
    memset(p->pname,'\0',k);
    memset(p->pvalue,'\0',v);
    strcpy(p->pname,key);
    strcpy(p->pvalue,value);
    return 0;
}
int destroy_param(struct http_param p)
{
    free(p.pname);
    free(p.pvalue);
    return 0;
}

bool paramIsNotNull(const struct http_param p)
{
    return !paramComp(p,l_error_param);
}

bool paramComp(const struct http_param p,const struct http_param d)
{
    return (p.pname == d.pname) && (p.pvalue == d.pvalue);
}

int clear_param(struct http_param* p)
{
    int i =0;
    
    while(paramIsNotNull(*(p+i)))
    {
        destroy_param(*(p+i));
        i++;
    }
    return 0;
}

char* http_paramGetValue(struct http_param* p,char *key)
{
    int i = 0;
    while(paramIsNotNull(*(p+i)))
    {
        if(strcmp(p->pname,key) == 0)
            return p->pvalue;
    }
    return 0;
}

struct http_param* parser(char *buffer, size_t s, char separator, char setter)
{
    //Checking if the request have a good termination
    /*
    if(buffer[s-1] != '\n' || buffer[s-2] != '\r' || buffer[s-3] != '\n' || buffer[s-4] != '\r')
    {
        printf("buffer does not finish with two new line\n");
        return -1;
    }
    */
    
    char key[125] = {'\0'};
    char value[125] = {'\0'};
    struct http_param* p;
    
    int poslinestart = 0;
    int poscolon = 0;
    bool firstcharmet = 0;
    int k = 0;          //size of malloc for param
    int j = 0;          //src index
    int l = 0;          //Dest index
    int i = 0;          //Buffer index (real reader)
    bool linec = 0;     //Boolean if contain setter

    p = malloc(sizeof(struct http_param)*(k+1));
    if(p == NULL)
    {
        return &l_error_param;
    }
    
    while(i < s)
    {
        if(buffer[i] == setter && linec != 1)
        {
            linec = 1;
            poscolon = i;
        }
        else if(buffer[i] == separator)
        {
            if(i > 0 && buffer[i-1] == '\r')
                i--;
            if(linec==1)
            {
                if(i > 0 && poscolon > 0 && poscolon < i-1 && poslinestart < poscolon)
                {
                    memset(key,'\0',125);
                    memset(value,'\0',125);
                    //Parse the line
                    if((poscolon - poslinestart) + 1 < 125 && (i - poscolon) + 2 < 125)
                    {
                        j = poslinestart;
                        l = 0;
                        firstcharmet=0;
                        //Taking the key
                        while(j<poscolon)
                        {
                            //Left Trim
                            if(firstcharmet != 0 || buffer[j] != ' ')
                            {
                                key[l]=buffer[j];
                                l++;
                                firstcharmet = 1;
                            }
                            j++;                            
                        }
                        //key[l] = '\0';
                        j=poscolon+1;
                        l = 0;
                        firstcharmet=0;
                        //Taking the value
                        while(j<i)
                        {
                            //Left Trim
                            if(firstcharmet != 0 || buffer[j] != ' ')
                            {
                                value[l]=buffer[j];
                                l++;
                                firstcharmet = 1;
                            }
                            j++;  
                        }
                        //value[l] = '\0';
                        
                        //KEY and Value are SET !!! Initialise
                        
                        init_param(key, value,p+k);
                        k++;
                        p = realloc(p,sizeof(struct http_param)*(k+1));
                    }
                }
            }
            linec=0;
            if(buffer[i] == '\r')
                i++;
            poslinestart=i+1;
        }
        i++;
    }
    p[k] = l_error_param;
    //free(p);
    return p;
}


list_param list_param_init()
{
	list_param ret = { .last = 0, .next = 0, .len = 0 };
	return ret;
}
pnode_list_param list_param_addlast(list_param * obj, http_param val)
{
	pnode_list_param ret;
	ret = malloc(sizeof(node_list_param));
	if(ret != NULL)
	{
		ret->next = 0;
		ret->value = val;
		if(obj->last == 0 || obj->next == 0)
		{
			obj->next = ret;
		}
		else
		{
			obj->last->next = ret;
		}
		obj->last = ret;
		(obj->len)++;
	}
	return ret;

}
pnode_list_param list_param_at(list_param * obj, int index)
{
	if(obj->len < index)
		return 0;
	pnode_list_param ret = obj->next;
	for(int i = 0; i < index; i++)
		ret = ret->next;
	return ret;
}
int list_param_free(list_param * obj)
{

}
