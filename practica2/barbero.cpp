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


void cortarPeloACliente(){
   chrono::milliseconds cortar_pelo( aleatorio<20,200>() );


   mtx.lock();
   cout << "El barbero comienza a cortar el pelo al cliente  " << endl;
   mtx.unlock();

   this_thread::sleep_for( cortar_pelo );

   mtx.lock();
   cout << "El barbero acaba de cortar el pelo al cliente " << endl;
   mtx.unlock();
}


class Barberia : public HoareMonitor{
private:

   CondVar barbero;
   CondVar silla;
   CondVar cola_espera;

public:
   Barberia();
   void cortarPelo(int n_cliente);
   void siguienteCliente();
   void finCliente();
};

Barberia::Barberia(){
   barbero = newCondVar();
   silla = newCondVar();
   cola_espera = newCondVar();
}

void Barberia::cortarPelo(int n_cliente){
   cout << "Entra cliente " << n_cliente << endl;
   if (cola_espera.empty()){
      if (barbero.empty()){
         cout << "Cliente " << n_cliente << " espera a que el barbero acabe" << endl;
         cola_espera.wait();
      }else{
         cout << "Cliente " << n_cliente << " despierta al barbero" << endl;
         barbero.signal();

      }
   }else{
      cout << "El cliente " << n_cliente << " se pone a la cola" << endl;
      cola_espera.wait();
   }
   cout << "Cliente " << n_cliente << " se pone en la silla " << endl;
   silla.wait();

}

void Barberia::siguienteCliente(){
   if (cola_espera.empty()){
      cout << "No hay clientes, el barbero se va a dormir" << endl;
      barbero.wait();
   }else{
      cout << "Avisa al siguiente cliente" << endl;
      cola_espera.signal();
   }
}

void Barberia::finCliente(){
   cout << "Barbero acaba con el cliente de la silla "<< endl;
   silla.signal();
}


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

   barbero = thread (funcion_hebra_barbero, barberia);


   thread clientes[NUM_CLIENTES];

   for (int i = 0; i < NUM_CLIENTES; i++)
      clientes[i] = thread (funcion_hebra_cliente, barberia, i);

   barbero.join();


   for (int i = 0; i < NUM_CLIENTES; i++)
      clientes[i].join();

}
