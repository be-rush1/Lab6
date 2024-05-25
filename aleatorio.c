#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUMERO_MAXIMO 1000
#define TAMANHO_ARQUIVO 1000 // Número de números a serem gerados

int main()
{
  FILE *arquivo = fopen("numeros.bin", "wb");
  if (!arquivo)
  {
    printf("Erro ao abrir o arquivo.\n");
    return 1;
  }

  srand(time(NULL));

  int numero;
  for (int i = 0; i < TAMANHO_ARQUIVO; i++)
  {
    numero = rand() % (NUMERO_MAXIMO + 1);
    fwrite(&numero, sizeof(int), 1, arquivo);
  }

  fclose(arquivo);
  printf("Arquivo 'numeros.bin' gerado com sucesso.\n");

  return 0;
}
