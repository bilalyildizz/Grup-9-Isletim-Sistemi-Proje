#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<signal.h>
#include<sys/wait.h>



int inputKontrolu(char* input,int *kelimeSayisi)
{
  // Fonksiyon icerisinde kullanicidan input aliniyor ve inputun gerekli kurallara uyup uymadigi kontrol ediliyor.
  
  
  int kelimeArasiBoslukSayisi=0;
  int komutBoslukSayisi=0;

 // Her komut satirinda bu sekilde sau yaziyor.
  printf("/: sau > ");
  fgets(input, 84, stdin);

	if (strlen(input) != 0) {
  
		int inputUzunlugu = strlen(input);
		
		if(inputUzunlugu>81){
			printf("En fazla 80 karakter girebilirsiniz");
			return 1;
		}
		
		if(input[0]==' '){
			printf("Komut oncesi bosluk olamaz");
			return 1;
		}		
		
		for(int i = 0; i<strlen(input); i++){
			if(input[i] != ' '){
				kelimeArasiBoslukSayisi=0;
			} 
			else if(input[i] == ' '){
				kelimeArasiBoslukSayisi++;
			}
			if(kelimeArasiBoslukSayisi>1){
				printf("Kelimeler arası en fazla 1 bosluk birakilabilir");
				return 1;
			}

			if(input[strlen(input)-2]==' '){
				printf("Komut sonunda boşluk bırakılamaz");
				return 1;
		}
	   
   }

    for(int i = 0; i<strlen(input); i++){
		if(input[i]==' '){
			 komutBoslukSayisi++;
		}
	}
		if(komutBoslukSayisi>9){
			printf("En fazla 10 kelime yazilabilir");
			return 1;
		}
		*kelimeSayisi=komutBoslukSayisi+1;
		return 0;
	 
	} 
	else{
		return 1;
	}
   

}
  
//getcwd ile icinde bulundugumuz dizine ulasabiliyoruz. Bu fonksiyon icinde bulundugumuz konumu ekrana yazdiriyor.
void Konum()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nKonum: %s", cwd);
}
  
//processs olusturularak girilen komut calistiriliyor.
void processOlustur(char** parsed)
{

   
    pid_t pid = fork(); 
	pid_t wpid;
	 int status;
	
	//pid 0 dan kucuksa process olusumunda hata oldugu anlasiliyor.
    if (pid <0) {
        printf("\nProcess olusumunda hata");
        return;
    } 
	//pid 0 esit ise cocuk proces olusmus demek.Execvp ile procese farkli bir gorev veriliyor.
	else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nKomut calistirilamiyor...");
        }
        exit(0);
    }
	// waitpid ile anne proces cocuk procesin sonlanmasini bekliyor.	
	else {
       do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}


//girilen komut bult in komutu  ise  onu calistiriyor.
int komutuCalistir(char** komut, int *kelimeSayisi)
{
	int toplamHarfSayisi=0;
	
	int a =strlen(komut[0]);
    int   eslesenKomut = 0;
	//built in komutlari
    char* builtInKomutlar[2]={"exit","cd"};
	

    for (int i = 0; i < 2; i++) {
        if (strcmp(komut[0], builtInKomutlar[i]) == 0) {
            eslesenKomut = i + 1;
            break;
        }
    }
 
//girilen komut exit ise shell sonlandiriliyor.
  if(eslesenKomut==1){
	  
	  exit(0);
  }
 //girilen komut cd ise ona gore icinde bulundugumuz dizin chdir ile degistiriliyor.
  else if(eslesenKomut==2){
	  if(*kelimeSayisi>2){
		   for (int i = 1; i < *kelimeSayisi; i++) {
			   int len =strlen(komut[i]);
		   toplamHarfSayisi += len;
		   }
		
		  char *result = malloc(2*toplamHarfSayisi);
		   for (int i = 1; i < *kelimeSayisi; i++) {
			   
			   if(i==1){
				   strcpy(result,  komut[i]);
				   
				   continue;
			   }
			 
				  
				   strcat(result, " "); 
				   strcat(result, komut[i]);
			   
			   
		     
		   }
		
		  chdir(result);
	  }
	  else
	  chdir(komut[1]);
	  
	  return 1;
  }


  
    return 0;
}
  


//Komut parcalanarak parametreler ayirt edilebilmesi saglaniyor.
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **komutuParcala(char *komut)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(komut, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

  
int main()
{
	char **parcalanmisKomut;
    char input[84];
	int kelimeSayisi=0;
  
  //Shell surekli olarak calismasi icin while(1) kullanildi.
    while (1) {

		//ilk basta icinde bulundugumuz dizin yazdiriliyor.
        Konum();
		
		//kullanicidan input alinarak kontroller yapiliyor. Problem var ise hata belirilip tekrar input bekleniyor.
		int sonuc = inputKontrolu(input,&kelimeSayisi);
        if (sonuc==1)
            continue;
		
		//girilen input parcalara ayrilarak diziye atiliyor.
		parcalanmisKomut =komutuParcala(input );
		
		//built in komutu ise komut calistiriliyor.
		int komutSonucu =komutuCalistir(parcalanmisKomut, &kelimeSayisi);

		//built in komutu degil ise process olusturulmasi icin process olustur klasorne gonderiliyor.
        if (komutSonucu == 0)
            processOlustur(parcalanmisKomut);

    }
    return 0;
}