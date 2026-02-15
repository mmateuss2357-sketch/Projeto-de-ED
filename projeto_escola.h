#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==========================================================================
   1. ESTRUTURAS DE DADOS (MODELAGEM HIERÁRQUICA)
   ========================================================================== */

// Notas organizadas por Unidade (Bimestre)
typedef struct {
    float prova1;
    float prova2;
    float media_unidade;
} Unidade;

// Entidade Professor: Nó da Lista Encadeada Global de Docentes
typedef struct Professor {
    char id[15];               // Identificador Único (ex: KOLP-01)
    char nome[100];
    char departamento[50];     // Área de atuação (ex: Exatas, Humanas)
    char email_funcional[150]; // Gerado automaticamente: nome.id@docente.kolping.edu.br
    struct Professor *proximo; // Ponteiro para o próximo professor na lista global
} Professor;

// Entidade Disciplina: Nó da Lista Encadeada Secundária (Cada aluno tem a sua)
typedef struct Disciplina {
    char nome[50];
    /* AVISO DE ARQUITETURA: 'docente' aponta para um endereço na Lista Global.
       Sempre use 'desvincular_professor_alunos' antes de dar free em um Professor. */
    Professor *docente;         
    Unidade unidades[4];        // Array fixo para os 4 bimestres
    float media_final;          // Média aritmética das 4 unidades
    struct Disciplina *proximo; // Ponteiro para a próxima disciplina da grade
} Disciplina;

// Entidade Aluno: Nó da Lista Encadeada Principal
typedef struct Aluno {
    char matricula[20];         // Matrícula alfanumérica única
    char nome[100];
    char email_academico[150];  // Gerado automaticamente: matricula.nome@kolping.edu.br
    int serie;                  
    Disciplina *lista_disciplinas; // Início da lista de matérias deste aluno
    struct Aluno *proximo;      // Próximo aluno (seja na lista global ou na turma)
} Aluno;

// Entidade Turma: Container que organiza o encontro de Alunos e Professores
typedef struct Turma {
    char codigo[10];            // Identificador da sala (ex: 6ANO-A)
    int serie;
    int limite_vagas;           // Lotação máxima da sala
    int qtd_atual;              // Contador de alunos matriculados
    Professor *professor_regente; // Professor responsável pela turma
    Aluno *lista_alunos;          // Início da lista de alunos desta turma
    struct Turma *proximo_turma;  // Próximo nó na lista global de turmas
} Turma;

/* ==========================================================================
   2. DADOS PREDEFINIDOS (GRADE CURRICULAR KOLPING)
   ========================================================================== */

const char *DISCIPLINAS_FUNDAMENTAL[] = {"Portugues", "Matematica", "Historia", "Geografia", "Ciencias", "Ingles", "Artes", "Educacao Fisica"};
const char *DISCIPLINAS_MEDIO[] = {"Portugues", "Matematica", "Historia", "Geografia", "Fisica", "Quimica", "Biologia", "Ingles", "Filosofia", "Sociologia"};

/* ==========================================================================
   3. GESTÃO DE PROFESSORES (CRUD & SEGURANÇA)
   ========================================================================== */

// Cria um professor na memória e gera seu e-mail institucional
Professor* criar_professor(char *id, char *nome, char *depto) {
    Professor *novo = (Professor*) malloc(sizeof(Professor));
    if (!novo) return NULL;
    strcpy(novo->id, id);
    strcpy(novo->nome, nome);
    strcpy(novo->departamento, depto);
    sprintf(novo->email_funcional, "%s.%s@docente.kolping.edu.br", novo->nome, novo->id);
    novo->proximo = NULL;
    return novo;
}

// Adiciona o professor no início da lista global (O(1))
void inserir_professor_global(Professor **cabeca, Professor *novo) {
    if (!novo) return;
    novo->proximo = *cabeca;
    *cabeca = novo;
}

// Localiza um professor pelo seu ID único
Professor* buscar_professor(Professor *cabeca, char *id_procurado) {
    Professor *atual = cabeca;
    while (atual) {
        if (strcmp(atual->id, id_procurado) == 0) return atual;
        atual = atual->proximo;
    }
    return NULL;
}

// NOVO: Função de Segurança contra 'Dangling Pointers'
// Antes de remover um professor, esta função percorre todos os alunos e limpa os vínculos
void desvincular_professor_alunos(Aluno *lista_alunos, Professor *p_removido) {
    Aluno *a_atual = lista_alunos;
    while (a_atual != NULL) {
        Disciplina *d_atual = a_atual->lista_disciplinas;
        while (d_atual != NULL) {
            if (d_atual->docente == p_removido) {
                d_atual->docente = NULL; // O aluno agora está "sem professor" nesta matéria
            }
            d_atual = d_atual->proximo;
        }
        a_atual = a_atual->proximo;
    }
}

// Remove um professor da lista global e libera a memória
void remover_professor_global(Professor **cabeca, char *id_remover) {
    Professor *atual = *cabeca, *anterior = NULL;
    while (atual && strcmp(atual->id, id_remover) != 0) {
        anterior = atual;
        atual = atual->proximo;
    }
    if (!atual) return;
    if (!anterior) *cabeca = atual->proximo;
    else anterior->proximo = atual->proximo;
    
    printf("Sistema Kolping: Memoria do docente %s liberada.\n", atual->nome);
    free(atual);
}

void listar_professores(Professor *cabeca) {
    printf("\n--- LISTA DE DOCENTES KOLPING ---\n");
    while (cabeca) {
        printf("ID: %-10s | Nome: %-20s | Depto: %s\n", cabeca->id, cabeca->nome, cabeca->departamento);
        cabeca = cabeca->proximo;
    }
}

/* ==========================================================================
   4. GESTÃO DE ALUNOS E TURMAS
   ========================================================================== */

// Matricula o aluno e já aloca todas as disciplinas baseadas na série
Aluno* matricular_aluno(char *matricula, char *nome, int serie) {
    Aluno *novo = (Aluno*) malloc(sizeof(Aluno));
    if (!novo) return NULL;
    strcpy(novo->matricula, matricula);
    strcpy(novo->nome, nome);
    novo->serie = serie;
    sprintf(novo->email_academico, "%s.%s@kolping.edu.br", novo->matricula, novo->nome);
    novo->lista_disciplinas = NULL;
    novo->proximo = NULL;

    int qtd = (serie >= 10) ? 10 : 8;
    const char **nomes = (serie >= 10) ? DISCIPLINAS_MEDIO : DISCIPLINAS_FUNDAMENTAL;

    for (int i = 0; i < qtd; i++) {
        Disciplina *d = (Disciplina*) malloc(sizeof(Disciplina));
        strcpy(d->nome, nomes[i]);
        d->docente = NULL;
        for (int u = 0; u < 4; u++) {
            d->unidades[u].prova1 = d->unidades[u].prova2 = d->unidades[u].media_unidade = 0;
        }
        d->proximo = novo->lista_disciplinas;
        novo->lista_disciplinas = d;
    }
    return novo;
}

Aluno* buscar_aluno(Aluno *cabeca, char *mat) {
    while (cabeca) {
        if (strcmp(cabeca->matricula, mat) == 0) return cabeca;
        cabeca = cabeca->proximo;
    }
    return NULL;
}

// Liberação de memória em cascata: Primeiro as disciplinas, depois o aluno
void deletar_aluno(Aluno *aluno) {
    if (!aluno) return;
    Disciplina *atual = aluno->lista_disciplinas;
    while (atual) {
        Disciplina *temp = atual;
        atual = atual->proximo;
        free(temp);
    }
    free(aluno);
}

Turma* criar_turma(char *codigo, int serie, int vagas) {
    Turma *nova = (Turma*) malloc(sizeof(Turma));
    if (!nova) return NULL;
    strcpy(nova->codigo, codigo);
    nova->serie = serie;
    nova->limite_vagas = vagas;
    nova->qtd_atual = 0;
    nova->professor_regente = NULL;
    nova->lista_alunos = NULL;
    nova->proximo_turma = NULL;
    return nova;
}

void inserir_turma_lista(Turma **lista_global, Turma *nova) {
    if (!nova) return;
    nova->proximo_turma = *lista_global;
    *lista_global = nova;
}

// Adiciona aluno na turma respeitando o limite físico de vagas
void vincular_aluno_turma(Turma *t, Aluno *a) {
    if (t->qtd_atual >= t->limite_vagas) {
        printf("ALERTA: Turma %s lotada! %s deve aguardar vaga.\n", t->codigo, a->nome);
        return;
    }
    a->proximo = t->lista_alunos;
    t->lista_alunos = a;
    t->qtd_atual++;
}

/* ==========================================================================
   5. OPERAÇÕES ACADÊMICAS (PORTAL DO DOCENTE)
   ========================================================================== */

// Vincula um professor da lista global a uma disciplina específica de um aluno
void atribuir_professor(Aluno *a, char *nome_materia, Professor *p) {
    if (!a || !p) return;
    Disciplina *atual = a->lista_disciplinas;
    while (atual != NULL) {
        if (strcmp(atual->nome, nome_materia) == 0) {
            atual->docente = p;
            return;
        }
        atual = atual->proximo;
    }
}

// Lança notas e calcula automaticamente a média da unidade
void lancar_nota(Aluno *lista, char *mat, char *materia, int unidade, int prova, float nota) {
    Aluno *a = buscar_aluno(lista, mat);
    if (!a) return;
    Disciplina *d = a->lista_disciplinas;
    while (d) {
        if (strcmp(d->nome, materia) == 0) {
            if (unidade < 1 || unidade > 4) return;
            if (prova == 1) d->unidades[unidade-1].prova1 = nota;
            else d->unidades[unidade-1].prova2 = nota;
            
            // Média Simples por Unidade
            d->unidades[unidade-1].media_unidade = (d->unidades[unidade-1].prova1 + d->unidades[unidade-1].prova2) / 2.0;
            return;
        }
        d = d->proximo;
    }
}

// Percorre as disciplinas e exibe as médias finais e professores vinculados
void exibir_boletim(Aluno *a) {
    if (!a) return;
    printf("\n========= BOLETIM KOLPING: %s (%s) =========\n", a->nome, a->matricula);
    Disciplina *d = a->lista_disciplinas;
    while (d) {
        float soma = 0;
        for(int i=0; i<4; i++) soma += d->unidades[i].media_unidade;
        d->media_final = soma / 4.0; // Média Final Anual
        
        printf("- %-15s | Media Final: %.2f | Prof: %s\n", 
                d->nome, d->media_final, d->docente ? d->docente->nome : "N/A");
        d = d->proximo;
    }
    printf("======================================================\n");
}



/* ==========================================================================
FILA DE ESPERA E CONTROLE DE VAGAS
   ========================================================================== */

// 1. ESTRUTURA DA FILA
// Usamos um nó específico para a fila que apenas "aponta" para o aluno,
// assim não quebramos o ponteiro "proximo" da lista encadeada original.
typedef struct NoFila {
    Aluno *aluno;
    struct NoFila *proximo;
} NoFila;

typedef struct FilaEspera {
    NoFila *inicio;
    NoFila *fim;
    int quantidade;
} FilaEspera;

// 2. FUNÇÕES BÁSICAS DA FILA (CRUD DA FILA)

FilaEspera* criar_fila() {
    FilaEspera *f = (FilaEspera*) malloc(sizeof(FilaEspera));
    f->inicio = NULL;
    f->fim = NULL;
    f->quantidade = 0;
    return f;
}

// Push/Enqueue - Insere o aluno no final da fila de espera
void enfileirar(FilaEspera *f, Aluno *a) {
    if (!f || !a) return;
    
    NoFila *novo = (NoFila*) malloc(sizeof(NoFila));
    novo->aluno = a;
    novo->proximo = NULL;

    if (f->fim == NULL) {
        f->inicio = novo; // Fila estava vazia
    } else {
        f->fim->proximo = novo; // Liga o último atual ao novo
    }
    f->fim = novo; // O novo passa a ser o último
    f->quantidade++;
    
    printf("FILA: %s adicionado a fila de espera (Posicao: %d).\n", a->nome, f->quantidade);
}

// Pop/Dequeue - Retira e retorna o aluno do início da fila
Aluno* desenfileirar(FilaEspera *f) {
    if (!f || f->inicio == NULL) return NULL; // Fila vazia

    NoFila *temp = f->inicio;
    Aluno *a_removido = temp->aluno; 

    f->inicio = f->inicio->proximo; 
    
    if (f->inicio == NULL) {
        f->fim = NULL; 
    }
    
    free(temp); // Libera o nó da fila, mas mantém o aluno intacto!
    f->quantidade--;
    
    return a_removido;
}

void exibir_fila(FilaEspera *f) {
    if (!f || f->inicio == NULL) {
        printf("\n[ Fila de Espera Vazia ]\n");
        return;
    }
    printf("\n--- FILA DE ESPERA (%d alunos aguardando) ---\n", f->quantidade);
    NoFila *atual = f->inicio;
    int pos = 1;
    while (atual != NULL) {
        printf("%dº lugar - Nome: %-20s | Mat: %s\n", pos++, atual->aluno->nome, atual->aluno->matricula);
        atual = atual->proximo;
    }
}

// Função que decide se o aluno entra na Turma ou vai para a Fila
void processar_matricula_turma(Turma *t, Aluno *a, FilaEspera *f) {
    if (t->qtd_atual < t->limite_vagas) {
        // Inserção na Lista Encadeada (Turma)
        a->proximo = t->lista_alunos;
        t->lista_alunos = a;
        t->qtd_atual++;
        printf("SUCESSO: %s matriculado na turma %s.\n", a->nome, t->codigo);
    } else {
        // Lotação atingida: vai para a Fila (Integrante 2 atua)
        printf("ALERTA: Turma %s lotada! ", t->codigo);
        enfileirar(f, a);
    }
}

// Função de remover da turma que puxa automaticamente o próximo da fila
void remover_aluno_turma(Turma *t, char *matricula, FilaEspera *f) {
    if (!t || !t->lista_alunos) return;

    Aluno *atual = t->lista_alunos;
    Aluno *anterior = NULL;

    while (atual != NULL && strcmp(atual->matricula, matricula) != 0) {
        anterior = atual;
        atual = atual->proximo;
    }

    if (atual == NULL) {
        printf("ERRO: Aluno %s nao encontrado.\n", matricula);
        return;
    }

    // Remove da lista
    if (anterior == NULL) {
        t->lista_alunos = atual->proximo;
    } else {
        anterior->proximo = atual->proximo;
    }
    t->qtd_atual--;
    printf("AVISO: %s foi removido da turma %s. Uma vaga abriu!\n", atual->nome, t->codigo);
    
    atual->proximo = NULL; 

    // Automação: Puxa o primeiro da fila para preencher a vaga
    if (f->quantidade > 0) {
        Aluno *promovido = desenfileirar(f);
        if (promovido) {
            printf(">> SISTEMA: Promovendo o proximo da fila de espera...\n");
            processar_matricula_turma(t, promovido, f); 
        }
    }
}