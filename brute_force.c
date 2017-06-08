#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LIM_INF '!'
#define LIM_SUP '~'


char *objetivo;

// Função que valida se a cadeia é a senha buscada
int validacao(char cadeia[]) {
	return (strcmp(cadeia, objetivo) == 0);
}

// Função que varia a string do inicio ao fim
double varia(char **senha, char lim_inf[], char lim_sup[]) {
	double iteracao = -1;
	
	// Descobre tamanho	
	size_t tamanho = strlen(lim_inf);

	// Copia inicio pra uma variável temporária
	char atual[tamanho];
	strcpy(atual, lim_inf);

	// Loop da variação
	while(strcmp(atual, lim_sup) != 0) {
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
		// Valida senha
		if(validacao(atual)) {
			// Aloca e retorna
			*senha = (char *) malloc(strlen(lim_inf) + 1);
			(*senha)[strlen(lim_inf)] = '\0';
			strcpy(*senha, atual);
			return iteracao;
		}

		iteracao++;
	}

	return -1;
}

int main() {
	objetivo = (char *) malloc(sizeof(char) * 155);
	printf("Insira a senha objetivo: ");
	scanf("%s", objetivo);

	char lim_inf[strlen(objetivo) + 1], lim_sup[strlen(objetivo) + 1];
	int i;
	for(i = 0; i < strlen(objetivo); i++) {
		lim_inf[i] = LIM_INF;
		lim_sup[i] = LIM_SUP;
	}
	lim_inf[strlen(objetivo)] = '\0';
	lim_sup[strlen(objetivo)] = '\0';


	char *senha = NULL;

	clock_t t_inicio, t_fim;
    double t_total;
     
    t_inicio = clock();
	double iteracao = varia(&senha, lim_inf, lim_sup); 
	t_fim = clock();
    t_total = ((double) (t_fim - t_inicio)) / CLOCKS_PER_SEC;

	if(iteracao != -1) {
		printf("Senha \"%s\" descoberta na iteração %.f em %f segundos\n", senha, iteracao, t_total);
	} else {
		printf("Senha não encontrada\n");
	}

}