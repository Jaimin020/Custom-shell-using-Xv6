//Myshell

#include "types.h"
#include "user.h"
#include "fcntl.h"

#define MAXARG 10
#define EXEC 1
#define PIPE 2
#define PAR  3
#define AND  4
#define OR   5

struct cmd
{
    int type;
    int ip;
    int op;
    char *ipf;
    char *opf;
    char *argv[MAXARG];
};


struct pcmd
{
    int type;
    struct cmd l;
    struct cmd r;
};

void parsecom(struct cmd* c,char *s);
char ls[100],rs[100];
char ipf[100],opf[100];
char buf[100];

int getcom(char *b,int bs)
{
    printf(2,"MyShell> ");
    //memset(b,0,bs);
    gets(b,bs);
    if(b[0]==0)
    {
        return -1;
    }
    return 0;
}

int gettok(char **ps, char *es,char **q)
{
    char *s = *ps;
    if(*s==0 || *s=='\0') return 0;

    while(s<es && strchr(" ",*s))
    {
        s++;
    }
    if(q)
        *q = s;
    while(s<es && !strchr(" ",*s))
    {
        s++;
    }
    if(*s==' ')
        *s='\0';
    s++;
    *ps = s;
    return 1;
}
void parsered(char *s, struct cmd *c)
{
    int ip=0;
    int op=0;
    int i=0;
    int n=strlen(s);

    while(i<n)
    {
        if(s[i]=='<')
        {
            s[i]='\0';
            i++;
            while(i<n)
            {
                if(s[i]=='>')
                {
                    break;
                }
                if(s[i]!=' ')
                {
                    ipf[ip] = s[i];
                    ip++;
                }
                i++;
            }
        }
        i++;
    }
    i=0;
    while(i<n)
    {
        if(s[i]=='>')
        {
            s[i]='\0';
            i++;
            while(i<n)
            {
                if(s[i]=='<')
                {
                    break;
                }
                if(s[i]!=' ')
                {
                    opf[op] = s[i];
                    op++;
                }
                i++;
            }
        }
        i++;
    }
    if(ip)
    {
        c->ip=1;
        c->ipf=ipf;
    }
    if(op)
    {
        c->op=1;
        c->opf=opf;
    }
}

void parsecom(struct cmd* c,char *s)
{
    char *ps;
    char *com;
    char *es = s + strlen(s);
    ps = s;
    int argc=0;
    c->type = EXEC;
    parsered(s,c);
    while(gettok(&ps,es,&com))
    {
        c->argv[argc] = com;
        argc++;
    }
}

int check_io(struct cmd* c)
{
    if(c->ip)
    {
        close(0);
        if(open(c->ipf,O_RDONLY)<0)
        {
            printf(2,"Invalid input file");
            return 0;
        }
    }
    if(c->op)
    {
        close(1);
        if(open(c->opf,O_WRONLY|O_CREATE)<0)
        {
            printf(2,"Invalid output file");
            return 0;
        }
    }
    return 1;
}

void runcom(struct cmd* c)
{
    int pip[2];
    switch (c->type)
    {
    case EXEC:
        check_io(c);   
        exec(c->argv[0],c->argv);
        break;
    
    case PIPE:
        struct pcmd *p=(struct pcmd*)c;
        if(pipe(pip)<0)
            printf(2,"Error in pipe\n");
        if(fork()==0)
        {
            close(1);
            dup(pip[1]);
            close(pip[0]);
            close(pip[1]);
            if(check_io(&(p->l)))
                exec(p->l.argv[0],p->l.argv);
        }
        if(fork()==0)
        {
            close(0);
            dup(pip[0]);
            close(pip[0]);
            close(pip[1]);
            if(check_io(&(p->r)))
                exec(p->r.argv[0],p->r.argv);
        }
        close(pip[0]);
        close(pip[1]);
        wait(0);
        wait(0);
        break;

    case PAR:
        struct pcmd *par=(struct pcmd*)c;
        if(fork()==0)
        {
            if(check_io((struct cmd*)&(par->l)))
                exec(par->l.argv[0],par->l.argv);
        }
        wait(0);
        if(check_io((struct cmd*)&(par->r)))   
            exec(par->r.argv[0],par->r.argv);
        break;
    
    case AND:
        struct pcmd *par2=(struct pcmd*)c;
        int status;
        if(fork()==0)
        {
            if(check_io((struct cmd*)&(par2->l)))
                exec(par2->l.argv[0],par2->l.argv);
        }
        wait(&status);
        if(status != -1)
        {
            if(check_io((struct cmd*)&(par2->r)))
                exec(par2->r.argv[0],par2->r.argv);
        }
        break;
    
    case OR:
        struct pcmd *par3=(struct pcmd*)c;
        int status1;
        if(fork()==0)
        {
            if(check_io((struct cmd*)&(par3->l)))
                exec(par3->l.argv[0],par3->l.argv);
        }
        wait(&status1);
        if(status1==-1)
        {
            if(check_io((struct cmd*)&(par3->r)))  
                exec(par3->r.argv[0],par3->r.argv);
        }
        break;
    
    default:
        break;
    }
    exit(0);
}

char check(char *b)
{
    int j=0;
    int n=strlen(b);
    while(j<n && !strchr("|;&",b[j]) )
    {
        j++;
    }
    if(b[j]=='|')
    {
        if(b[j+1]=='|')
        {
            return 'o';
        }
    }
    if(j==n)
    {
        return 'a';
    }
    return *(b+j);
}

int check_invali_com(char *ls,char *rs)
{
    if(ls[0]=='\0' || ls[0]==0)
    {
        printf(2,"Illegal command or arguments\n");
        return 0;
    }
    if(rs[0]=='\0' || rs[0]==0)
    {
        printf(2,"Illegal command or arguments\n");
        return 0;
    }
    return 1;
}

int parsepip(struct pcmd* p,char *b,char a)
{
    p->type = PIPE;
    int i=0;
    int n=strlen(b);
    while(i<n && b[i]!=a)
    {
        ls[i]=b[i];
        i++;
    }
    ls[i]='\0';
    i++;
    int j=0;
    while(i<n)
    {
        rs[j]=b[i];
        i++;
        j++;
    }
    rs[j]='\0';
    parsecom(&(p->l),ls);
    parsecom(&(p->r),rs);
    return check_invali_com(ls,rs);
}

void nullter(char *b)
{
    int j=0;
    while(b[j]!='\n')
    {
        if(b[j]=='\0') return;
        j++;
    }
    b[j]='\0';
}

int parseand(struct pcmd* p,char *b,char a)
{
    p->type = AND;
    int i=0;
    int n=strlen(b);
    while(i<n && b[i]!=a)
    {
        ls[i]=b[i];
        i++;
    }
    ls[i]='\0';
    i=i+2;
    int j=0;
    while(i<n)
    {
        rs[j]=b[i];
        i++;
        j++;
    }
    parsecom(&(p->l),ls);
    parsecom(&(p->r),rs);
    return check_invali_com(ls,rs);
}

int parseor(struct pcmd* p,char *b,char a)
{
    p->type = OR;
    int i=0;
    int n=strlen(b);
    while(i<n && b[i]!=a)
    {
        ls[i]=b[i];
        i++;
    }
    ls[i]='\0';
    i=i+2;
    int j=0;
    while(i<n)
    {
        rs[j]=b[i];
        i++;
        j++;
    }
    parsecom(&(p->l),ls);
    parsecom(&(p->r),rs);
    return check_invali_com(ls,rs);
}

void clear_mem()
{
    for(int i=0;i<100;i++)
    {
        ls[i]='\0';
        rs[i]='\0';
        ipf[i]='\0';
        opf[i]='\0';
        buf[i]='\0';
    }
}

void init_argv(char *c[])
{
    for(int i=0;i<MAXARG;i++)
    {
        c[i]='\0';
    }
}

void filter(char* buf)
{
    nullter(buf);
            char x = check(buf);
            if(x=='a')
            {
                struct cmd comm;
                comm.ip=0;
                comm.op=0;
                init_argv(comm.argv);
                parsecom(&comm,buf);
                if(fork()==0)
                    runcom(&comm);
                wait(0);
            }
            else{
                switch (x)
                {
                    case '|':
                        struct pcmd comm;
                        comm.l.ip=0;
                        comm.l.op=0;
                        comm.r.ip=0;
                        comm.r.op=0;
                        init_argv(comm.l.argv);
                        init_argv(comm.r.argv);
                        if(parsepip(&comm,buf,x) && fork()==0)
                            runcom((struct cmd*)&comm);
                        wait(0);
                        break;
                    
                    case ';':
                        struct pcmd comm1;
                        comm1.l.ip=0;
                        comm1.l.op=0;
                        comm1.r.ip=0;
                        comm1.r.op=0;
                        init_argv(comm1.l.argv);
                        init_argv(comm1.r.argv);

                        if(parsepip(&comm1,buf,x) && fork()==0)
                        {
                            comm1.type=PAR;
                            runcom((struct cmd*)&comm1);
                        }
                        wait(0);
                        break;

                    case '&':
                        struct pcmd comm2;
                        comm2.l.ip=0;
                        comm2.l.op=0;
                        comm2.r.ip=0;
                        comm2.r.op=0;
                        init_argv(comm2.l.argv);
                        init_argv(comm2.r.argv);

                        if(parseand(&comm2,buf,x) && fork()==0)
                            runcom((struct cmd*)&comm2);
                        wait(0);
                        break;
                    
                    case 'o':
                        struct pcmd comm3;
                        comm3.l.ip=0;
                        comm3.l.op=0;
                        comm3.r.ip=0;
                        comm3.r.op=0;
                        init_argv(comm3.l.argv);
                        init_argv(comm3.r.argv);

                        if(parseor(&comm3,buf,'|') && fork()==0)
                            runcom((struct cmd*)&comm3);
                        wait(0);
                        break;
                    
                    default:
                        break;
                }
            }
}

void sep(char *c,int n)
{
    char t[100];
    int i=0;
    int j=0;
    while(i<n)
    {
        if(c[i]=='\n')
        {
            c[i]='\0';
            t[j]='\0';
            j=0;
            filter(t);
            clear_mem();
            for(int i=0;i<100;i++)
            {
                t[i]='\0';
            }
        }
        else
        {
            t[j++]=c[i];
        }
        i++;
    }
}

int main()
{
    //static char buf[100];
    int fd;
    // Ensure that three file descriptors are open.
    while((fd = open("console", O_RDWR)) >= 0){
        if(fd >= 3){
        close(fd);
        break;
        }
    }

    while(getcom(buf,sizeof(buf))>=0)
    {
        if(buf[0]=='e' && buf[1]=='x' && buf[2]=='i' && buf[3]=='t')
        {
            exit(0);
        }
        else if(buf[0]=='e' && buf[1]=='x' && buf[2]=='e')
        {
            char buf1[100];
            nullter(buf);
            char *fname=buf+16;
            int fd=open(fname,O_RDONLY);
            if(fd<0)
            {
                printf(2,"Invalid file");
            }
            int n;
            n = read(fd,buf1,sizeof(buf1)); 
            for(int i=0;i<100;i++)
            {
                buf[i]=0;
            }
            sep(buf1,n); 
        }
        else
        {
            filter(buf);
        }
        clear_mem();
    }
    exit(0);
}