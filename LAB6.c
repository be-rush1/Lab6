#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define TAMANHO_MAX_BUFFER 10

typedef struct
{
  int *buffer;
  int tamanho;
  int entrada;
  int saida;
  int contador;
  pthread_mutex_t mutex;
  pthread_cond_t cheio;
  pthread_cond_t vazio;
} Buffer;

typedef struct
{
  int id;
  Buffer *buffer;
  int primos_encontrados;
} ArgsConsumidor;

void inicializar_buffer(Buffer *buffer, int tamanho)
{
  buffer->buffer = (int *)malloc(sizeof(int) * tamanho);
  buffer->tamanho = tamanho;
  buffer->entrada = 0;
  buffer->saida = 0;
  buffer->contador = 0;
  pthread_mutex_init(&buffer->mutex, NULL);
  pthread_cond_init(&buffer->cheio, NULL);
  pthread_cond_init(&buffer->vazio, NULL);
}

void destruir_buffer(Buffer *buffer)
{
  free(buffer->buffer);
  pthread_mutex_destroy(&buffer->mutex);
  pthread_cond_destroy(&buffer->cheio);
  pthread_cond_destroy(&buffer->vazio);
}

void colocar_item(Buffer *buffer, int item)
{
  pthread_mutex_lock(&buffer->mutex);
  while (buffer->contador >= buffer->tamanho)
  {
    pthread_cond_wait(&buffer->cheio, &buffer->mutex);
  }
  buffer->buffer[buffer->entrada] = item;
  buffer->entrada = (buffer->entrada + 1) % buffer->tamanho;
  buffer->contador++;
  pthread_cond_signal(&buffer->vazio);
  pthread_mutex_unlock(&buffer->mutex);
}

int pegar_item(Buffer *buffer)
{
  int item;
  pthread_mutex_lock(&buffer->mutex);
  while (buffer->contador <= 0)
  {
    pthread_cond_wait(&buffer->vazio, &buffer->mutex);
  }
  item = buffer->buffer[buffer->saida];
  buffer->saida = (buffer->saida + 1) % buffer->tamanho;
  buffer->contador--;
  pthread_cond_signal(&buffer->cheio);
  pthread_mutex_unlock(&buffer->mutex);
  return item;
}

int eh_primo(long long int n)
{
  int i;
  if (n <= 1)
    return 0;
  if (n == 2)
    return 1;
  if (n % 2 == 0)
    return 0;
  for (i = 3; i <= sqrt(n); i += 2)
    if (n % i == 0)
      return 0;
  return 1;
}

void *consumidor(void *args)
{
  ArgsConsumidor *cargs = (ArgsConsumidor *)args;
  int primos_encontrados = 0;
  int item;
  while ((item = pegar_item(cargs->buffer)) != -1)
  {
    if (eh_primo(item))
    {
      primos_encontrados++;
    }
  }
  cargs->primos_encontrados = primos_encontrados;
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    printf("Uso: %s <num_consumidores> <tamanho_buffer> <arquivo_entrada>\n", argv[0]);
    return 1;
  }

  int num_consumidores = atoi(argv[1]);
  int tamanho_buffer = atoi(argv[2]);
  char *arquivo_entrada = argv[3];

  FILE *arquivo = fopen(arquivo_entrada, "rb");
  if (!arquivo)
  {
    printf("Erro ao abrir o arquivo.\n");
    return 1;
  }

  Buffer buffer;
  inicializar_buffer(&buffer, tamanho_buffer);

  pthread_t *consumidores = (pthread_t *)malloc(sizeof(pthread_t) * num_consumidores);
  ArgsConsumidor *args = (ArgsConsumidor *)malloc(sizeof(ArgsConsumidor) * num_consumidores);

  for (int i = 0; i < num_consumidores; i++)
  {
    args[i].id = i;
    args[i].buffer = &buffer;
    args[i].primos_encontrados = 0;
    pthread_create(&consumidores[i], NULL, consumidor, (void *)&args[i]);
  }

  int num;
  while (fread(&num, sizeof(int), 1, arquivo))
  {
    colocar_item(&buffer, num);
  }
  // Sinaliza o fim dos dados colocando -1 no buffer para cada consumidor
  for (int i = 0; i < num_consumidores; i++)
  {
    colocar_item(&buffer, -1);
  }

  fclose(arquivo);

  for (int i = 0; i < num_consumidores; i++)
  {
    pthread_join(consumidores[i], NULL);
  }

  int total_primos = 0;
  int max_primos = 0;
  int vencedor = -1;
  for (int i = 0; i < num_consumidores; i++)
  {
    printf("Consumidor %d encontrou %d primos.\n", i, args[i].primos_encontrados);
    total_primos += args[i].primos_encontrados;
    if (args[i].primos_encontrados > max_primos)
    {
      max_primos = args[i].primos_encontrados;
      vencedor = i;
    }
  }
  printf("Total de primos encontrados: %d\n", total_primos);
  printf("Consumidor vencedor: %d\n", vencedor);

  destruir_buffer(&buffer);
  free(consumidores);
  free(args);

  return 0;
}
