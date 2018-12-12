#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_procesos  = 2*num_filosofos + 1,
   id_camarero = num_procesos - 1,
   etiq_levantarse = 2,
   etiq_sentarse = 1 ;

const int num_asientos = num_filosofos - 1;


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------
void funcion_camarero(){
	int num_filosofos_mesa = 0, etiq_valor, valor;
	MPI_Status estado;
	while (true){

		if (num_filosofos_mesa < num_asientos){
			etiq_valor = MPI_ANY_TAG;
		}else{
			etiq_valor = etiq_levantarse;
		}

		MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_valor, MPI_COMM_WORLD, &estado);

		if ( estado.MPI_TAG  == etiq_sentarse){
			num_filosofos_mesa++;
			MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE , etiq_sentarse, MPI_COMM_WORLD);
		} else if (estado.MPI_TAG == etiq_levantarse){
			num_filosofos_mesa--;
		}

	}


}


void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % num_procesos, //id. tenedor izq.
      id_ten_der = (id+num_procesos-1) % num_procesos , //id. tenedor der.
      valor = 0;
  MPI_Status estado;

  while ( true )
  {
	 cout <<"Filósofo " <<id << " solicita sentarse  al camarero." <<endl << flush;
    MPI_Ssend( &valor, 1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD);

    MPI_Recv ( &valor, 1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD, &estado);
    cout <<"Camarero sento al filosofo " <<id << " en la mesa." <<endl << flush;

    cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl << flush;
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl << flush;
    // ... solicitar tenedor derecho (completar)
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

    sleep_for( milliseconds( aleatorio<10,15>() ) );
    cout <<"\t\t\tFilósofo " <<id <<" comienza a comer" <<endl << flush;
    sleep_for( milliseconds( aleatorio<100,1000>() ) );

    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl << flush;
    // ... soltar el tenedor izquierdo (completar)
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl << flush;
    // ... soltar el tenedor derecho (completar)
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id << " solicita levantarse al camarero." <<endl << flush;
    MPI_Ssend( &valor, 1, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD);
    cout <<"Camarero levanto al filosofo " <<id << " de la mesa." <<endl << flush;

    cout << "Filosofo " << id << " comienza a pensar" << endl << flush;
    sleep_for( milliseconds( aleatorio<100,1000>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones
  while ( true )
  {
     // ...... recibir petición de cualquier filósofo (completar)
     // ...... guardar en 'id_filosofo' el id. del emisor (completar)
     MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado);
     id_filosofo = estado.MPI_SOURCE;
     cout <<"\tTen. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl << flush;


     // ...... recibir liberación de filósofo 'id_filosofo' (completar)
     MPI_Recv( &valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado);
     cout <<"\tTen. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl << flush;
  }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
	  if(id_propio == id_camarero){
		  funcion_camarero();
	  }else{
		  // ejecutar la función correspondiente a 'id_propio'
		  if ( id_propio % 2 == 0 )          // si es par
			 funcion_filosofos( id_propio ); //   es un filósofo
		  else                               // si es impar
			 funcion_tenedores( id_propio ); //   es un tenedor
	  }
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl << flush;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
