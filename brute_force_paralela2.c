#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mpi.h>


#define LIM_INF_GLOBAL '!'
#define LIM_SUP_GLOBAL '~'

#define MESTRE 0


char *objetivo;

// Função que valida se a cadeia é a senha buscada
int validacao(char cadeia[]) {
	return (strcmp(cadeia, objetivo) == 0);
}

// Função que varia a string do inicio ao fim
double varia(char **senha, char lim_inf[], char lim_sup[]) {
	double iteracao = 0;
	MPI_Request req;
	
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

		// Se alguma thread já achou a resposta, ela retorna
		//int achou = 0;
		//MPI_Irecv(&achou, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req);
		//if(achou) {
		//	return -1;
		//}
	}
	if(validacao(atual)) {
		// Aloca e retorna
		*senha = (char *) malloc(strlen(lim_inf) + 1);
		(*senha)[strlen(lim_inf)] = '\0';
		strcpy(*senha, atual);
		return iteracao;
	}

	return -1;
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
	MPI_Init(NULL, NULL);

	int n_processos;
	MPI_Comm_size(MPI_COMM_WORLD, &n_processos);

	int id_processo;
	MPI_Comm_rank(MPI_COMM_WORLD, &id_processo);

	objetivo = (char *) malloc(155 * sizeof(char));
	// Só om o malloc ele reclama que a mensagem ta truncada, se fizer o strcpy ele não reclama
	// Deve ser algum erro de alocação de ponteiro, não sei qual
	strcpy(objetivo, "0123456789");
	if(id_processo == MESTRE) {
		printf("Insira a senha objetivo: ");
		fflush(stdout);
		scanf("%s", objetivo);
	}


	MPI_Bcast(objetivo, strlen(objetivo) + 1, MPI_BYTE, MESTRE, MPI_COMM_WORLD);

	// Gera o vetor que vai gerar os limites inferiores e superiores
	int i;
	int tamanho_intervalo = LIM_SUP_GLOBAL - LIM_INF_GLOBAL + 1;
	char *intervalo = (char *) malloc(sizeof(char) * tamanho_intervalo);
	if(id_processo == MESTRE) {
		for(i = 0; i < tamanho_intervalo; i++) {
			intervalo[i] = i + LIM_INF_GLOBAL;
		}
	}

	// Cria o array de distribuição do Scatterv
	int *distribuicao = (int *) malloc(sizeof(int) * n_processos);
	distribuicao = distribui(n_processos);

	// Cria o array de displs(offsets) pro Scatterv
	int displs[n_processos];
	int offset = 0;
	for(i = 0; i < n_processos; i++) {
		displs[i] = offset;
		offset += distribuicao[i];

	}

	// Distribui os limites pras funções
	char *limites = malloc(sizeof(char) * distribuicao[id_processo]);
	MPI_Scatterv(intervalo, distribuicao, displs, MPI_BYTE, limites, distribuicao[id_processo], MPI_BYTE, 0, MPI_COMM_WORLD);

	// Gera os limites inferior e superior
	char lim_inf = limites[0];
	char lim_sup = limites[distribuicao[id_processo] - 1];

	// Constroi as strings de limite e envia pra função
	char str_lim_inf[strlen(objetivo) + 1], str_lim_sup[strlen(objetivo) + 1];
	for(i = 0; i < strlen(objetivo); i++) {
		str_lim_inf[i] = lim_inf;
		str_lim_sup[i] = lim_sup;
	}
	str_lim_inf[strlen(objetivo)] = '\0';
	str_lim_sup[strlen(objetivo)] = '\0';


	char *senha = NULL;

	clock_t t_inicio, t_fim;
    double t_total;
     
    t_inicio = clock();
	double iteracao = varia(&senha, str_lim_inf, str_lim_sup); 
	t_fim = clock();
    t_total = ((double) (t_fim - t_inicio)) / CLOCKS_PER_SEC;

	if(iteracao != -1) {
		// Avisa às outras threads que já achou
		printf("Senha \"%s\" descoberta pela processo %d na iteração %.f em %f segundos\n", senha, id_processo, iteracao, t_total);
		MPI_Abort(MPI_COMM_WORLD,1);
	}

	MPI_Finalize();
}
