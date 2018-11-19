#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo


#include "HoareMonitor.h"


using namespace std ;
using namespace HM;

const int NUM_CLIENTES = 3;

mutex mtx;


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


void EsperarFueraBarberia(int num_cliente){
   chrono::milliseconds espera_fuera( aleatorio<20,200>() );


   mtx.lock();
   cout << "Cliente " << num_cliente << "  :"
         << " sale de la barberia (" << espera_fuera.count() << " milisegundos)" << endl;
   mtx.unlock();

   this_thread::sleep_for( espera_fuera );

   mtx.lock();
   cout << "Cliente " << num_cliente << "  : termina la espera." << endl;
   mtx.unlock();
}


void EsperarFueraBarberia(int num_cliente){
   chrono::milliseconds cortar_pelo( aleatorio<20,200>() );


   mtx.lock();
   cout << "El barbero comienza a cortar el pelo al cliente  " << num_cliente << endl;
   mtx.unlock();

   this_thread::sleep_for( cortar_pelo );

   mtx.lock();
   cout << "El barbero acaba de cortar el pelo al cliente " << num_cliente << endl;
   mtx.unlock();
}


class Barberia : public HoareMonitor{
private:
   CondVar barbero;
   CondVar clientes[NUM_CLIENTES];

public:
   Barberia();
   void cortarPelo(int n_cliente);
   void siguienteCliente();
   void finCliente();
};


//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_barbero( MRef<Barberia> barberia )
{


   while (true){
      barberia->siguienteCliente();
      cortarPeloACliente();
      barberia->finCliente();

   }
}


//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_cliente( MRef<Barberia> barberia, int num_cliente )
{
   while( true )
   {

      barberia->cortarPelo(num_cliente);
      EsperarFueraBarberia(num_cliente);

   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   // ......

   MRef<Barberia> barberia = Create<Barberia> ();

   thread barbero;

   thread clientes[NUM_CLIENTES];

   for (int i = 0; i < NUM_CLIENTES; i++)
      clientes[i] = thread (funcion_hebra_cliente, barberia, i);

   barbero = thread (funcion_hebra_barbero, barberia);

   for (int i = 0; i < NUM_CLIENTES; i++)
      clientes[i].join();

   barbero.join();
}
