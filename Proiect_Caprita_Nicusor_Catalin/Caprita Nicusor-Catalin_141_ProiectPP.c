#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef struct {unsigned char  b,g,r;} Pixel;
typedef struct { Pixel **Smat;
                double medie,sigma;
                } Sablon;
typedef struct {unsigned int X,Y;} Colt;
typedef struct {double corelatie,medie,sigma;
                Colt sus, jos;
                Pixel culoare;} Detectie;

void Liniarizare(char *cale_input,Pixel **L,unsigned int *img_w,unsigned int *img_l,unsigned int *padding,unsigned char **head,unsigned int *img_size)
{
    FILE *fin=fopen(cale_input,"rb");
    if(fin==NULL)
    {
        printf("Eroare la identificarea fisierului!\n");
        return;
    }
    fseek(fin,18,SEEK_SET);
    fread(img_w,4,1,fin);
    fread(img_l,4,1,fin);
    (*head)=(unsigned char *)malloc(54);
    fseek(fin,0,SEEK_SET);
    fread((*head),1,54,fin);
    int low,high,k,i,j;
    (*img_size)=(*img_l)*(*img_w);
    (*L)=(Pixel *)malloc((*img_size)*sizeof(Pixel));
    Pixel p;
    low=0;high=(*img_size)-(*img_w);
    //calculez padding
    if((*img_w)%4!=0)
        (*padding)=4-((*img_w)*3)%4;
    else
        (*padding)=0;
    for(i=0;i<(*img_l);i++)
    {
        for(j=0;j<(*img_w);j++)
        {
            //citesc un pixel
            fread(&p.b,1,1,fin);
            fread(&p.g,1,1,fin);
            fread(&p.r,1,1,fin);
            //incarc in vectorul de pixeli, pixelul pe pozitia corespunzatoare, ordonat
            (*L)[(*img_w)*i+j].b=p.b;
            (*L)[(*img_w)*i+j].g=p.g;
            (*L)[(*img_w)*i+j].r=p.r;
        }
        //Sar peste padding
        fseek(fin,(*padding),SEEK_CUR);
    }
//    Inversez Pozitiile
    while(low<high)
    {
        for(k=0;k<(*img_w);k++)
        {
            Pixel paux=(*L)[low+k];
            (*L)[low+k]=(*L)[high+k];
            (*L)[high+k]=paux;
        }
        low+=(*img_w);
        high-=(*img_w);
    }
       fclose(fin);
}
void Deliniarizare( Pixel *L,char *cale_output,unsigned int img_l,unsigned int img_w,unsigned int padding,unsigned char *head,unsigned int img_size)
{
    FILE *fout=fopen(cale_output,"wb");
    if(fout==NULL)
    {
        printf("Eroare la crearea fisierului!");
        return;
    }
    //copiez header;
   fwrite(head,1,54,fout);
   int block,k,i;
    block=img_size-img_w;
    while(block>=0)
    {
        k=block;
        for(i=0;i<img_w;i++)
        {
            fwrite(&L[k].b,1,1,fout);
            fwrite(&L[k].g,1,1,fout);
            fwrite(&L[k].r,1,1,fout);
            k++;
        }
        char c=0;
        fwrite(&c,1,padding,fout);
        block-=img_w;
    }
    fclose(fout);
}
unsigned int *XORSHIFT(unsigned int seed,int n)
{
    unsigned int x=seed,i;
    unsigned int *R=(unsigned int *)malloc(n*sizeof(unsigned int));
    R[0]=seed;
    for(i=0;i<n;i++)
    {
        x^=x<<13;
        x^=x>>17;
        x^=x<<5;
        R[i]=x;
    }
    return R;
}
unsigned int *Permutare(int n,unsigned int *r)
{
    unsigned int *Sigma=(unsigned int*)malloc(n*sizeof(unsigned int));
    int i=0;
    for(i=0;i<n;i++)
        Sigma[i]=i;
    for(i=n-1;i>0;i--)
    {
        int j=r[n-i]%(i+1);
        //if(j!=i)
            {
                unsigned int aux=Sigma[i];
                Sigma[i]=Sigma[j];
                Sigma[j]=aux;
            }
    }
    return Sigma;
}
Pixel CifrareInt(Pixel p,unsigned int R)
{
        unsigned char mask=255&R;
            p.b^=mask;
            R=R>>8;
            mask=255&R;
            p.g^=mask;
            R=R>>8;
            mask=255&R;
            p.r^=mask;
        return p;
}
Pixel CifrarePix(Pixel p1, Pixel p2)
{
    p1.b^=p2.b;
    p1.g^=p2.g;
    p1.r^=p2.r;
    return p1;
}
void CriptareImagine(char *nume_sursa, char *nume_dest, char *nume_cheie,unsigned int **Sigma, unsigned int **R )
{
    Pixel *L;
    unsigned int img_l,img_w,padding,i,img_size;
    unsigned char *head=NULL;
    Liniarizare(nume_sursa,&L,&img_w,&img_l,&padding,&head,&img_size);
   printf("Imaginea are dimensiunea %d x %d\nImaginea are %d octeti de padding\n",img_w,img_l,padding);
    FILE *fkey=fopen(nume_cheie,"r");
    if(fkey==NULL)
    {
        printf("Eroare la deschiderea fisierului text!");
        return;
    }
    unsigned int SV,R0;
    fscanf(fkey,"%u %u",&R0,&SV);
    //Generez Numere Random
    (*R)=XORSHIFT(R0,2*img_size);
    //Generez Permutarea Random
    (*Sigma)=Permutare(img_size,(*R));
    //Permut Pixelii din L in Intermediara L2
    Pixel *L2=(Pixel *)malloc((img_size*sizeof(Pixel)));
    for(i=0;i<img_size-1;i++)
        L2[(*Sigma)[i]]=L[i];
    //Salvez in C imaginea Criptata
    Pixel *C=(Pixel *)malloc((img_size*sizeof(Pixel)));
    //Aplic Cifrarea
    C[0]=CifrareInt(L2[0],SV);
    C[0]=CifrareInt(C[0],(*R)[img_size]);
    for(i=1;i<img_size;i++)
        {
            C[i]=CifrarePix(C[i-1],L2[i]);
            C[i]=CifrareInt(C[i],(*R)[img_size+i]);
        }
   Deliniarizare(C,nume_dest,img_l,img_w,padding,head,img_size);
    fclose(fkey);
}
void DecriptareImagine(char *nume_sursa, char* nume_dest, char *nume_cheie,unsigned int *Sigma, unsigned int *R)
{
    Pixel *C;
    unsigned int img_l,img_w,padding,i,img_size;
    unsigned char *head=NULL;
    //Liniarizez in C imaginea criptata
    Liniarizare(nume_sursa,&C,&img_w,&img_l,&padding,&head,&img_size);
    FILE *fkey=fopen(nume_cheie,"r");
    if(fkey==NULL)
    {
        printf("Eroare la deschiderea fisierului text!");
        return;
    }
    //Calculez Imaginea Intermediara in C2
    unsigned int SV,R0;
    unsigned int *Sigma2=(unsigned int *)malloc(img_size*sizeof(unsigned int));
    Pixel *D=(Pixel* )malloc(img_size*sizeof(Pixel));
    Pixel *C2=(Pixel* )malloc(img_size*sizeof(Pixel));
    fscanf(fkey,"%u %u",&R0,&SV);
    //Calculez inversul permutarii
    for(i=0;i<img_size;i++)
        Sigma2[Sigma[i]]=i;
    //Inversul Criptarii
    C2[0]=CifrareInt(C[0],SV);
    C2[0]=CifrareInt(C2[0],R[img_size]);
    for(i=1;i<img_size;i++)
        {
            C2[i]=CifrarePix(C[i-1],C[i]);
            C2[i]=CifrareInt(C2[i],R[img_size+i]);
        }
    //Permut Intermediara conform Sigma2 si C2
    for(i=0;i<img_size;i++)
        D[Sigma2[i]]=C2[i];
//    //Deliniarizez D in memoria externa
    Deliniarizare(D,nume_dest,img_l,img_w,padding,head,img_size);
}
void ChiSquared(char *cale_input)
{
    FILE *fin=fopen(cale_input,"rb");
    double rtest=0,gtest=0,btest=0;
    unsigned int *frb=(unsigned int *)calloc(256,sizeof(unsigned int ));
    unsigned int *frg=(unsigned int *)calloc(256,sizeof(unsigned int ));
    unsigned int *frr=(unsigned int *)calloc(256,sizeof(unsigned int ));
    unsigned int img_l,img_w,padding,i,img_size,j;
    fseek(fin,18,SEEK_SET);
    fread(&img_w,4,1,fin);
    fread(&img_l,4,1,fin);
    fseek(fin,54,SEEK_SET);
    img_size=img_l*img_w;
    if(img_w%4!=0)
    padding=4-(img_w*3)%4;
    else
        padding=0;
    double f=img_size/256;
    Pixel p;//=(Pixel )malloc((sizeof(Pixel)));
    for(i=0;i<img_l;i++)
    {
        for(j=0;j<img_w;j++)
        {
            fread(&p,sizeof(Pixel),1,fin);
            frb[p.r]++;
            frg[p.g]++;
            frr[p.b]++;
        }
        fseek(fin,padding,SEEK_CUR);
    }
    for(i=0;i<256;i++)
    {
        rtest=rtest+(double)((frr[i]-f)*(frr[i]-f))/f;
        btest=btest+(double)((frb[i]-f)*(frb[i]-f))/f;
        gtest=gtest+(double)((frg[i]-f)*(frg[i]-f))/f;
    }
    printf("\nValorile testului Chi Patrat pentru imaginea aleasa sunt:\nR: %.2lf\nG: %.2lf\nB: %.2lf",rtest,gtest,btest);
    fclose(fin);
    free(frr);
    free(frg);
    free(frb);
}
void Greyscale( char *cale_input,char *cale_output)
{
    FILE *fin=fopen(cale_input,"rb");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea fisierului sursa!;");
        return;
    }
    FILE *fout=fopen(cale_output,"wb+");
    if(fout==NULL)
    {
        printf("Eroare la deschiderea fisierului destinatie!;");
        return;
    }
    Pixel p;
    unsigned int i,j;
    unsigned int img_w,img_l,padding;
    fseek(fin,18,SEEK_SET);
    fread(&img_w,sizeof(int),1,fin);
    fread(&img_l,sizeof(int),1,fin);
    //calculez padding
    if(3*img_w%4!=0)
        padding=4-(img_w*3)%4;
    else
        padding=0;
    printf("Imaginea este de dimensiune %d x %d si are padding de %d\n",img_w,img_l,padding);
    fseek(fin,0,SEEK_SET);
    unsigned char c;
	while(fread(&c,1,1,fin)==1)
	{
		fwrite(&c,1,1,fout);
		fflush(fout);
	}
	fseek(fout, 54, SEEK_SET);
    for(i=0;i<img_l;i++)
    {
        for(j=0;j<img_w;j++)
            {
                fread(&p.b,1,1,fout);
                fread(&p.g,1,1,fout);
                fread(&p.r,1,1,fout);
                fseek(fout,-3,SEEK_CUR);
                unsigned char aux= 0.299*p.b + 0.587*p.g + 0.114*p.r;
                p.r = p.g = p.b =aux;
                fwrite(&p.b,3,1,fout);
                fflush(fout);
            }
        fseek(fout,padding,SEEK_CUR);
    }
    fclose(fin);
    fclose(fout);
}

Sablon *Matricealizare(char *cale_imagine,unsigned int img_w,unsigned int img_l,unsigned int padding)
{
   FILE *fin=fopen(cale_imagine,"rb");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea imaginii sablon!;");
        return 0;
    }

    Sablon *s=(Sablon *)malloc(sizeof(Sablon));
    s->Smat=(Pixel **)malloc(img_l*sizeof(Pixel*));
    s->medie=0;
    s->sigma=0;
    unsigned int i,j;
    fseek(fin,54,SEEK_SET);
    for(i=0;i<img_l;i++)
    {
        s->Smat[i]=(Pixel *)malloc(img_w*sizeof(Pixel));
        for(j=0;j<img_w;j++)
        {
            fread(&s->Smat[i][j],3,1,fin);
            s->medie+=s->Smat[i][j].b;
        }
    fseek(fin,padding,SEEK_CUR);
    }
    s->medie=s->medie/(img_l*img_w);
    fclose(fin);
    for(i=0;i<img_l;i++)
        for(j=0;j<img_w;j++)
        s->sigma=s->sigma+(s->Smat[i][j].b-s->medie)*(s->Smat[i][j].b-s->medie);
        s->sigma=(double)sqrt(s->sigma/(img_l*img_w-1));
    return s;
}
void Medie_Sigma(Detectie *d,Sablon *I,unsigned int x,unsigned int y,unsigned int temp_w,unsigned int temp_l)
{
    unsigned int i,j;
    d->medie=0;
    d->sigma=0;
    for(i=0;i<temp_l;i++)
        for(j=0;j<temp_w;j++)
            d->medie+=I->Smat[y+i][x+j].b;
    d->medie=(double)d->medie/(temp_l*temp_w);
    for(i=0;i<temp_l;i++)
        for(j=0;j<temp_w;j++)
        d->sigma=d->sigma+(I->Smat[y+i][x+j].b-d->medie)*(I->Smat[y+i][x+j].b-d->medie);
        d->sigma=(double)sqrt(d->sigma/(temp_l*temp_w-1));
}
void Colorare(char *cale_input,Detectie d,unsigned int temp_w,unsigned int temp_l,unsigned int img_w,unsigned int img_l,unsigned int padding)
{
    FILE *fin=fopen(cale_input,"rb+");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea fisierului!");
        return;
    }
    fseek(fin,54,SEEK_SET);
    fseek(fin,3*(d.sus.Y*(img_w+padding)+d.sus.X),SEEK_CUR);
    unsigned i,j;
    for(j=0;j<temp_w;j++)
        fwrite(&d.culoare,3,1,fin);
    for(i=1;i<temp_l;i++)
    {
        fseek(fin,3*(img_w-temp_w+padding),SEEK_CUR);
        fwrite(&d.culoare,3,1,fin);
        fseek(fin,3*(temp_w-2),SEEK_CUR);
        fwrite(&d.culoare,3,1,fin);
    }
    fseek(fin,3*(img_w-temp_w+padding),SEEK_CUR);
    for(j=0;j<temp_w;j++)
        fwrite(&d.culoare,3,1,fin);
    fclose(fin);
}
void Template_Match(char *cale_input,char *cale_input_grey,char *cale_sablon,char *cale_sablon_grey,double ps,Detectie **D,unsigned int *D_size,Pixel Culoare)
{
    FILE *fin=fopen(cale_input,"rb");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea imaginii sursa!");
        return;
    }
    unsigned int img_w,img_l,img_padding;
    fseek(fin,18,SEEK_SET);
    fread(&img_w,sizeof(int),1,fin);
    fread(&img_l,sizeof(int),1,fin);
    //calculez padding
    if(3*img_w%4!=0)
        img_padding=4-(img_w*3)%4;
    else
        img_padding=0;
    FILE *fin2=fopen(cale_sablon,"rb");
    if(fin2==NULL)
    {
        printf("Eroare la deschiderea imaginii sablon!");
        return;
    }
    unsigned int temp_w,temp_l,temp_padding;
    fseek(fin2,18,SEEK_SET);
    fread(&temp_w,sizeof(int),1,fin2);
    fread(&temp_l,sizeof(int),1,fin2);
    //calculez padding
    if(3*temp_w%4!=0)
        temp_padding=4-(temp_w*3)%4;
    else
        temp_padding=0;
    //unsigned int temp_l=15,temp_w=11,temp_padding=3,img_l=400,img_w=500,img_padding=0;
    Sablon *S,*I;
    Greyscale(cale_sablon,cale_sablon_grey);
    //Matricealizez Sablonul S si imaginea I pentru a obtine valorile Pixelilor
    S=Matricealizare(cale_sablon,temp_w,temp_l,temp_padding);
    I=Matricealizare(cale_input_grey,img_w,img_l,img_padding);
    //Calculul corelatiei
    unsigned int x,y,i,j,k=0;
    for(y=0;y<=img_l-temp_l;y++)
    {
        for(x=0;x<=img_w-temp_w;x++)
    {
        Detectie *dcrt=(Detectie *)malloc(sizeof(Detectie));
        dcrt->sus.X=x;dcrt->sus.Y=y;
        dcrt->jos.X=x+temp_w;dcrt->jos.Y=y+temp_l;
        dcrt->corelatie=0;
        dcrt->culoare=Culoare;
        Medie_Sigma(dcrt,I,x,y,temp_w,temp_l);
    //Calculez corelatia
    for(i=0;i<temp_l;i++)
        for(j=0;j<temp_w;j++)
            dcrt->corelatie=dcrt->corelatie+(double)((I->Smat[y+i][x+j].b-dcrt->medie)*(S->Smat[i][j].b-S->medie))/(dcrt->sigma*S->sigma);
        dcrt->corelatie=(double)dcrt->corelatie/(temp_l*temp_w);
    //Daca gasesc corelatie mai mare ca ps,introduc in vectorul de detectii
    if(dcrt->corelatie>ps)
    {
        (*D_size)++;
        Detectie *aux=(Detectie *)realloc((*D),(*D_size)*sizeof(Detectie));
        aux[(*D_size)-1]=(*dcrt);
        (*D)=aux;
    }}}
    fclose(fin);
    fclose(fin2);
}
int Comparator(const void *a,const void *b)
{
    Detectie va=*(Detectie *)a;
    Detectie vb=*(Detectie *)b;
    if(va.corelatie<vb.corelatie)
        return 1;
    else
        if(va.corelatie==vb.corelatie)
        return 0;

    return -1;
}
unsigned int max(unsigned int a,unsigned int b)
{
    if(a>b) return a;
    return b;
}
unsigned int min(unsigned int a,unsigned int b)
{
    if(a<b) return a;
    return b;
}
double Suprapunere(Detectie d1,Detectie d2)
{
    Colt intersus, interjos;
    intersus.X=0;intersus.Y=0;
    interjos.X=0;interjos.Y=0;
    intersus.X=min(d1.sus.X,d2.sus.X);
    intersus.Y=min(d1.sus.Y,d2.sus.Y);
    interjos.X=max(d1.jos.X,d2.jos.X);
    interjos.Y=max(d1.jos.Y,d2.jos.Y);
    if(intersus.X>interjos.X || intersus.Y>interjos.Y)return 0;
    double aria_inter=(interjos.X-intersus.X)*(interjos.Y-intersus.Y);
    double aria_d=(d1.jos.X-d1.sus.X)*(d1.jos.Y-d1.sus.Y);
    return (aria_inter/((2*aria_d)-aria_inter));
}
void Eliminare_Non_Maxime(Detectie **D,unsigned int *D_size)
{
    Detectie *Daux=(Detectie *)malloc(sizeof(Detectie));
    unsigned char *mark=(unsigned char *)calloc((*D_size),sizeof(Detectie));
    unsigned int i,j,aux_size=0;
    for(i=0;i<(*D_size);i++)
    {
        if(mark[i]==0)
        {
            for(j=i+1;j<(*D_size);j++)
            if(mark[j]==0 && Suprapunere((*D)[i],(*D)[j])>0.2)
            mark[j]++;
            aux_size++;
            Detectie *aux2=(Detectie *)realloc(Daux,aux_size*sizeof(Detectie));
            aux2[aux_size-1]=(*D)[i];
            Daux=aux2;
        }
    }
    *D=Daux;
    *D_size=aux_size;
}
unsigned int Cautare_Sablon(char cale_sablon_defeault[10][15],char *cale_sablon)
{
    unsigned int i=0;
    for(i=0;i<10;i++)
    {
        if(strcmp(cale_sablon,cale_sablon_defeault[i])==0)
            return i;
    }
    return -1;
}
int main()
{
    char cale_input_criptare[256];
    char cale_criptata[]="Imagine_Criptata.bmp";
    char cale_decriptata[]="Imagine_Decriptata.bmp";
    char cale_cheie[256];

    printf("Numele Imaginii ce trebuie Criptate este:");
    fgets(cale_input_criptare,256,stdin);
    cale_input_criptare[strlen(cale_input_criptare)-1]='\0';

    printf("Numele Fisierului ce contine cheia secreta este:");
    fgets(cale_cheie,256,stdin);
    cale_cheie[strlen(cale_cheie)-1]='\0';

    Pixel *L=NULL;
    unsigned int *R,*Sigma;
    CriptareImagine(cale_input_criptare,cale_criptata,cale_cheie,&Sigma,&R);
    DecriptareImagine(cale_criptata,cale_decriptata,cale_cheie,Sigma,R);
    ChiSquared(cale_criptata);

    char cale_input_pattern[256];
    char cale_sablon_defeault[10][15]={"cifra0.bmp","cifra1.bmp","cifra2.bmp","cifra3.bmp","cifra4.bmp","cifra5.bmp","cifra6.bmp","cifra7.bmp","cifra8.bmp","cifra9.bmp"};
    char cale_fisier_date[256];
    unsigned int numar_sabloane;
    Pixel Culoare[10]={{0,0,255},{0,255,255},{0,255,0},{255,255,0},{255,0,255},{255,0,0},{192,192,192},{0,140,255},{128,0,128},{0,0,128}};
    printf("\nNumele Fisierului ce contine datele de intrare pentru operatia de pattern matching :");
    fgets(cale_fisier_date,256,stdin);
    cale_fisier_date[strlen(cale_fisier_date)-1]='\0';
    FILE *fin=fopen(cale_fisier_date,"r");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea fisierului cu datele de intrare!");
        return -1;
    }
    fgets(cale_input_pattern,256,fin);
    cale_input_pattern[strlen(cale_input_pattern)-1]='\0';
    printf("imaginea aleasa este : %s",cale_input_pattern);
    fscanf(fin,"%d\n",&numar_sabloane);
    char cale_input_grey[]="inputGrey.bmp";
    char cale_sablon_grey[]="sablonGrey.bmp";
    unsigned int img_l,img_w,img_padding;
     FILE *fin1=fopen(cale_input_pattern,"rb");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea imaginii sursa!");
        return;
    }
    fseek(fin1,18,SEEK_SET);
    fread(&img_w,sizeof(int),1,fin1);
    fread(&img_l,sizeof(int),1,fin1);
    //calculez padding
    if(3*img_w%4!=0)
        img_padding=4-(img_w*3)%4;
    else
        img_padding=0;
    Greyscale(cale_input_pattern,cale_input_grey);
    Detectie *D=(Detectie *)malloc(sizeof(Detectie));
    unsigned int D_size=0;
    unsigned int i,j;
   for(i=0;i<numar_sabloane;i++)
    {
        char cale_sablon_curent[15];
        fgets(cale_sablon_curent,15,fin);
        cale_sablon_curent[strlen(cale_sablon_curent)-1]='\0';
        unsigned int nrord=Cautare_Sablon(cale_sablon_defeault,cale_sablon_curent);
        unsigned int crt=D_size;
        Template_Match(cale_input_pattern,cale_input_grey,cale_sablon_curent,cale_sablon_grey,0.5,&D,&D_size,Culoare[nrord]);
        printf("Pentru culoarea %d,am gasit %d detectii!\n",nrord,D_size-crt);
    }
    qsort(D,D_size,sizeof(Detectie),Comparator);
    Eliminare_Non_Maxime(&D,&D_size);
    for(i=0;i<D_size;i++)
     Colorare(cale_input_grey,D[i],11,15,img_w,img_l,img_padding);
     fclose(fin);
     fclose(fin1);
    return 0;
}
