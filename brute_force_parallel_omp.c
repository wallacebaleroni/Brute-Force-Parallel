#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define LIM_INF_GLOBAL '!'
#define LIM_SUP_GLOBAL '~'

#define NAO_ENCONTRADA -1
#define DESNECESSARIA -2


char *objetivo;
int encontrada;

// Função que valida se a cadeia é a senha buscada
int validacao(char cadeia[]) {
	return (strcmp(cadeia, objetivo) == 0);
}

// Função que varia a string do inicio ao fim
double varia(char **senha, char lim_inf[], char lim_sup[]) {
	double iteracao = 0;
	
	// Descobre tamanho	
	size_t tamanho = strlen(lim_inf);

	// Copia inicio pra uma variável temporária
	char atual[tamanho];
	strcpy(atual, lim_inf);

	// Loop da variação
	while(strcmp(atual, lim_sup) != 0) {
		// Valida senha
		if(validacao(atual)) {
			// Aloca e retorna
			*senha = (char *) malloc(strlen(lim_inf) + 1);
			(*senha)[strlen(lim_inf)] = '\0';
			strcpy(*senha, atual);
			return iteracao;
		}

		// Incrementa o final da string
		atual[tamanho - 1]++;
		int i;
		// Percorre a string pra "rodar" os valores
		for(i = tamanho - 1; i >= 0; i--) {
			if(atual[i] == 127) {
				if(i == 0) {
					return 0;
				}
				atual[i] = 33;
				atual[i - 1]++;
			} else {
				i = -1;
				continue;
			}
		}

		iteracao++;
		
		if(encontrada) return DESNECESSARIA;
	}
	if(validacao(atual)) {
		// Aloca e retorna
		*senha = (char *) malloc(strlen(lim_inf) + 1);
		(*senha)[strlen(lim_inf)] = '\0';
		strcpy(*senha, atual);
		return iteracao;
	}

	return NAO_ENCONTRADA;
}

int somatorio(int n_processos, int distribuicao[n_processos]) {
	int i;
	int somatorio = 0;
	for(i = 0; i < n_processos; i++) {
		somatorio += distribuicao[i];
	}

	return somatorio;
}


int *distribui(int n_processos) {
	int *distribuicao = (int *) malloc(sizeof(int) * n_processos);
	int tamanho_intervalo = LIM_SUP_GLOBAL - LIM_INF_GLOBAL + 1;
	int i;
	for(i = 0; i < n_processos; i++) {
		distribuicao[i] = tamanho_intervalo / n_processos;
	}

	somatorio(n_processos, distribuicao);

	int iter = 0;
	while(somatorio(n_processos, distribuicao) < tamanho_intervalo) {
		for(i = 0; i < iter + 1; i++) {
			distribuicao[n_processos - i - 1] += 1;
		}
		iter++;
	}

	return distribuicao;
}

int main() {
	encontrada = 0;
	
	objetivo = (char *) malloc(sizeof(char) * 155);
	printf("Insira a senha objetivo: ");
	scanf("%s", objetivo);
	
	int n_threads;
	printf("Insira o numero de threads: ");
	scanf("%d", &n_threads);

	int i;
	int tamanho_intervalo = LIM_SUP_GLOBAL - LIM_INF_GLOBAL + 1;
	char *intervalo = (char *) malloc(sizeof(char) * tamanho_intervalo);
	for(i = 0; i < tamanho_intervalo; i++) {
		intervalo[i] = i + LIM_INF_GLOBAL;
	}

	// Cria o array de distribuição
	int *distribuicao = (int *) malloc(sizeof(int) * n_threads);
	distribuicao = distribui(n_threads);

	// Cria o array de posicoes de lim_sup
	char *senha = NULL; 
	int posicoes[n_threads];
	int soma = 0;
	for(i = 0; i < n_threads; i++){
		if(i == 0) {
			posicoes[i] = LIM_INF_GLOBAL;
			soma = LIM_INF_GLOBAL;
		} else {
			soma += distribuicao[i];
			posicoes[i] = soma;
		}
	}

	#pragma omp parallel num_threads(n_threads)
	{
		int id = omp_get_thread_num();
		char *limites = malloc(sizeof(char) * distribuicao[id]);
		
		char lim_inf = posicoes[id];
		char lim_sup;
		if(id == n_threads - 1) {
			lim_sup = LIM_SUP_GLOBAL;
		} else {
			lim_sup = posicoes[id + 1] - 1;
		}

		// Constroi as strings de limite e envia pra função
		char str_lim_inf[strlen(objetivo) + 1], str_lim_sup[strlen(objetivo) + 1];
		str_lim_inf[0] = lim_inf;
		str_lim_sup[0] = lim_sup;

		for(i = 1; i < strlen(objetivo); i++) {
			str_lim_inf[i] = LIM_INF_GLOBAL;
			str_lim_sup[i] = LIM_SUP_GLOBAL;
		}
		str_lim_inf[strlen(objetivo)] = '\0';
		str_lim_sup[strlen(objetivo)] = '\0';
		clock_t t_inicio, t_fim;
	    double t_total;

	    printf("thread: %d\nlim_inf: %s\nlim_sup: %s\n", id, str_lim_inf, str_lim_sup);
	     
	    t_inicio = clock();

		double iteracao = varia(&senha, str_lim_inf, str_lim_sup); 
		
		t_fim = clock();
	    t_total = ((double) (t_fim - t_inicio)) / CLOCKS_PER_SEC;

		if(iteracao != DESNECESSARIA) {
			if(iteracao != NAO_ENCONTRADA) {
	    		encontrada = 1;
				printf("Senha \"%s\" descoberta na iteração %.f em %f segundos\n", senha, iteracao, t_total);
			}
		}
	}
	if(!encontrada) {
		printf("Senha não encontrada\n");
	}
}
