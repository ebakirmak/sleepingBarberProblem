#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

//Customer'ların maksimum sayısı
#define MAX_CUSTOMERS 25
//Barber ve BarberChairs'ların maksimum sayısı
#define MAX_BARBERCHAIRS 5

//Fonksiyonların prototipleri
void *customer(void *num);
void *barber(void *);
void randwait(int secs);

// Semaphore'ları tanımlayacağız..Semaphore'ları kullanma amacımız belirli sayıdaki kaynağa erişimi denetlemektir.

// waitingRoom bir defada bekleme odasına girmesine izin verilen customer sayısını sınırlar.
sem_t waitingRoom;

//barberChair, berber koltuğuna tek bir customer'in oturabilmesini sağlar.
sem_t barberChair;

//barberPillow bir customer gelene kadar barber'ların uyumasına izin vermek için kullanılır.
sem_t barberPillow;

//seatBelt, barber'ların kişinin saçlarını kesene kadar customer'i beklettirmek için kullanılır.
sem_t seatBelt;

//Tüm customer'ların saçları kesildiğinde barber'ları eve göndermek için işaretliyoruz.
int allDone = 0;
int waitingRoomValue=-1;

int main(int argc, char *argv[]) {
    //Barber thread id
    //MAX_BARBERCHAIRS 5 olduğu için 5 barberChairs olur bu yüzden 5 barber olduğunu varsayıyoruz.
    pthread_t btid[MAX_BARBERCHAIRS];
    //Customers thread id
    pthread_t tid[MAX_CUSTOMERS];
    long RandSeed;
    int i, numCustomers, numChairs, numBarberChairs;
    int Number[MAX_CUSTOMERS];
    int NumberBarberChairs [MAX_BARBERCHAIRS];

    //Komut satırından girilen parametlerin doğruluğunu kontrol ediyoruz.
    if (argc != 5) {
    printf("Kullanim: Uyuyan Berber Problemi <Customer sayisi> <Bekleme odasındaki sandalye sayisi> <Berber koltugu sayisi> <Rastgele erisim>\n");
    exit(-1);
    }

    //Komut satırındaki argümanları ilgili değişkenlere atayarak convert işlemi yapıyoruz.
    numCustomers = atoi(argv[1]);
    numChairs = atoi(argv[2]);
    numBarberChairs = atoi(argv[3]);
    RandSeed = atol(argv[3]);
    waitingRoomValue=numChairs;
    //Komut satırından girilen customer sayısı tid thread dizisinin boyundan büyükse çıkış yap.
    if (numCustomers > MAX_CUSTOMERS) {
    printf("Maksimum müsteri sayisi %d'dir. Tekrar calistiriniz.\n", MAX_CUSTOMERS);
    exit(-1);
    }
    
    if (numBarberChairs > MAX_BARBERCHAIRS) {
    printf("Maksimum berber koltuk sayisi %d'dir. Tekrar calistiriniz.\n", MAX_BARBERCHAIRS);
    exit(-1);
    }


    printf("Uyuyan Berber Problemi\n\n");
    printf("Semaphore'lari kullanarak uyuyan berber problemi cozumu.\n");

    //Girilen parametreye göre rastgele sayı üretir.
    srand48(RandSeed);

    // Number dizisine customer no atar.
    for (i=0; i<MAX_CUSTOMERS; i++) {
    Number[i] = i;
    }
    //NumberBarberChairs dizisine barberChair no atar.
     for (i=0; i<MAX_BARBERCHAIRS; i++) {
    NumberBarberChairs[i] = i;
    }
    //sem_init fonksiyonu ile kaynak kullanımını denetlemeye başlayacağız.
    //sem_init(sem_t *sem, int pshared, unsigned int value); yapısındadır.
    //sem_t *sem => sem_t tipindeki ilgili semaphorumuz/kaynağımız.
    //int pshared=>Bu semaforun prosesin threadleri arasında mı yoksa prosesler arasında mı paylaşılacağını belirler. pshared 0 ise bu prosesin threadleri arasında paylaşılacağını gösterir.
    //int value=> Kaynak sayımızı belirtir.

    //Başlangıç değerleriyle semaphorları başlat.
    //numChairs kadar sandalye içeren bekleme odası
    sem_init(&waitingRoom, 0, numChairs);
    //numBarberChairs kadar barberChair içerir. 
    sem_init(&barberChair, 0, numBarberChairs);
    sem_init(&barberPillow, 0, 0);
    sem_init(&seatBelt, 0, 0);

     //int pthread_create(pthread_t tid, const pthread_attr_t *attr,	void *(*start_routine) (void *), void *arg);
     //parameter 1: pthread_t => thread ids
     //parameter 2: NULL => default attributes
     //parameter 3: start_routine => will run the related function

    // barber'ları yarat.	
    for(i=0;i<numBarberChairs;i++){
    pthread_create(&btid[i], NULL, barber,(void *)&NumberBarberChairs[i]);
    }

    // customers'ları yarat.
    for (i=0; i<numCustomers; i++) {    
    pthread_create(&tid[i], NULL, customer, (void *)&Number[i]);
    }

    //pthread_join fonksiyonu hedef/ilgili thread ölene kadar diğer thread çağrılarını askıya alır. Sıralı bir işlem garantisi sunar.
    // customer'ların/ customer threadlerinin sırayla bitirilmesini garantilemek için join fonk kullandık.
    for (i=0; i<numCustomers; i++) {
    pthread_join(tid[i],NULL);	
    }
	
    //Tüm customer threadleri bittiğinde, barber threadini öldür. 
    //sem_post fonk. ile barberPillow semaphorundaki kilit kaldırılır.
    allDone = 1;
  
    //sem_post(sem_t *sem) fonksiyonu sem tarafından işaret edilen semaphore'un değerini arttırır yani kilidi açar. Bu sayede diğer semaphore'lar için kaynak artmış olur ve sem_wait(sem_t *sem) fonksiyonu ile kilitlenebilir/kaynak kullanılabilir.
   waitingRoomValue=1;
    for(i=0;i<numBarberChairs;i++){
     sem_post(&barberPillow);
     pthread_join(btid[i],NULL);    
    }
    //pthread_join(btid[5],NULL);
	printf("PROGRAM SONLANDI.\n");
}

void *customer(void *number) {
	int num = *(int *)number;

	//Dükkan için ayrılın ve varmak için rastgele zaman
	printf("Müsteri %d berber dükkani icin evden cikti.\n", num);
	randwait(2);
	
	printf("Müsteri %d berber dukkanına vardi.\n", num);
	if(waitingRoomValue==0)
		printf("Müşteri %d dükkandan ayrıldı. ***Waiting Room'da yer olmadığı için.***\n",num);
	else
	{	
	//sem_wait(sem_t *sem)  sem tarafından işaret edilen semaphore'un değerini azaltır/kilitler. Eğer semaphore değeri 0'dan büyükse başka semaphore'lar tarafından sem_wait() fonksiyonu kullanılabilir. Semaphore değeri 0 olduğunda sem_wait() fonksiyonları engellenir. Semaphore değerinin artması için sem_post() fonksiyonu beklenir.

	// waitingRoom semaphoru bir customer için sem_wait fonk. ile kilitlendi.
	sem_wait(&waitingRoom);
	waitingRoomValue-=1;
	printf("Müsteri %d bekleme odasina girdi.\n", num);
	// barberChair semaphoru  bir customer için sem_wait fonk. ile kilitlendi.
	sem_wait(&barberChair);
	//customer barberChair'ı kilitledikten sonra, sem_post fonk. ile waitingRoom semaphorun'daki kilit kaldırılıyor.
	sem_post(&waitingRoom);
	waitingRoomValue+=1;
	// barber uyandırılıyor.
	printf("Müsteri %d berberi uyandirdi.\n", num);
	sem_post(&barberPillow);

	// Saç traşı bitene kadar sem_wait fonk. ile seatBelt semaphoru kilitleniyor.
	sem_wait(&seatBelt);

	// barberChair semaphorundaki kilit kaldırılıyor.
	sem_post(&barberChair);
	printf("Müsteri %d berber dukkanindan ayrildi.\n", num);
	}
	

	
}

void *barber(void *junk) {
	int jun = *(int *)junk;
	//Hizmet edilecek müşteri olduğu sürece
	while (!allDone) {
		//Biri varana kadar uyur ve varan kişi uyandırır.
		printf("Berber %d uyuyor.\n", jun);
		sem_wait(&barberPillow);
	
		// hizmet edilecek customer varsa traş devam et.
		if (!allDone) {
			
			//Tıraş için rastgele bir zaman harca
			printf("Berber %d sac kesimi yapıyor.\n", jun);
			randwait(3);
			printf("Berber %d sac kesimini bitirdi.\n", jun);

			// Saç tıraşı bittiğinde müşteriyi serbest bırak.
			sem_post(&seatBelt);
		}
		//customer yoksa barber evine gitsin.
		else {
			printf("Berber %d evine gitti.\n", jun);
			
		}
	}

			
	
}

void randwait(int secs) {
	int len;

	// Random sayı üretir
	//drand48 fonksiyonu [0.0] ve (1.0) arasında double sayılır üretir.
	len = (int)(((drand48()+0,1) * secs) + 1);
	sleep(len);
}


