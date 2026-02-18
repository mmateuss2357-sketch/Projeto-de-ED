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

/* ============================================================
   ESTRUTURA PILHA (DESFAZER AÇÕES)
   ============================================================ */

// Cada ação representa uma operação que pode ser desfeita
typedef struct Acao {
    char tipo[20];       // Tipo da ação: "nota", "professor_removido", "aluno_removido"
    void *dado;          // Ponteiro genérico para o dado que será restaurado
    struct Acao *proximo; 
} Acao;

// Pilha que mantém o topo das ações salvas
typedef struct Pilha {
    Acao *topo;           
} Pilha;

// Função para criar uma pilha vazia
Pilha* criarPilha() {
    Pilha *p = (Pilha*) malloc(sizeof(Pilha)); 
    if (!p) return NULL;                       
    p->topo = NULL;                             // Inicialmente, a pilha está vazia
    return p;
}

// Função para salvar uma ação na pilha antes de qualquer alteração
void salvar_acao(Pilha *p, char *tipo, void *dado) {
    if (!p || !tipo || !dado) return;          

    Acao *novo = (Acao*) malloc(sizeof(Acao)); 
    strcpy(novo->tipo, tipo);                  // Copia o tipo da ação
    novo->dado = dado;                         // Salva o ponteiro para o dado
    novo->proximo = p->topo;                   // Faz o novo apontar para o topo atual
    p->topo = novo;                            // Atualiza o topo da pilha para o novo nó
}

// Função para desfazer a última ação
void desfazer(Pilha *p) {
    if (!p || !p->topo) {                      
        printf("Nada para desfazer!\n");
        return;
    }

    Acao *a = p->topo;                         // Pega a ação do topo
    p->topo = a->proximo;                       // Atualiza o topo para o próximo

    // Dependendo do tipo, restaura-se o estado antigo
    if (strcmp(a->tipo, "nota") == 0) {
        Unidade *u = (Unidade*) a->dado;       // Recupera a unidade antiga
        
    } 
    else if (strcmp(a->tipo, "professor_removido") == 0) {
        Professor *p_restaurar = (Professor*) a->dado;
        // Reinsere o professor removido na lista global
    } 
    else if (strcmp(a->tipo, "aluno_removido") == 0) {
        Aluno *a_restaurar = (Aluno*) a->dado;
        // Reinsere o aluno removido na lista da turma
    }

    free(a);                                   // Libera a memória do nó da pilha 
}

/* ============================================================
   ESTRUTURA PILHA PARA HISTÓRICO DE MENUS
   ============================================================ */

// Cada nó representa uma tela/menu visitado
typedef struct Menu {
    int id_menu;          
    struct Menu *proximo; 
} Menu;


Menu *menu_topo = NULL;

// Empilha ao entrar em um menu
void entrar_menu(int id) {
    Menu *novo = malloc(sizeof(Menu));  
    if (!novo) return;                  
    novo->id_menu = id;                 // Guarda o ID do menu
    novo->proximo = menu_topo;          // Aponta para o menu anterior (topo atual)
    menu_topo = novo;                   // Atualiza o topo da pilha
}

// Desempilha ao voltar para o menu anterior
int voltar_menu() {
    if (!menu_topo) return -1;          // Se pilha vazia, retorna -1 (nenhum menu anterior)

    Menu *temp = menu_topo;             // Guarda o topo atual
    menu_topo = menu_topo->proximo;     // Atualiza o topo para o próximo da pilha
    int id = temp->id_menu;             // Recupera o ID do menu a retornar
    free(temp);                         // Libera a memória do nó desempilhado
    return id;                          // Retorna o ID do menu anterior
}

/* Valida se a nota esta entre 0.0 e 10.0.
   Retorna 1 se valida, 0 se invalida (com mensagem de erro). */
int validar_nota(float nota) {
    if (nota < 0.0f || nota > 10.0f) {
        printf("[ERRO] Nota invalida: %.2f. Deve estar entre 0.0 e 10.0.\n", nota);
        return 0;
    }
    return 1;
}

/* Valida se unidade (1-4) e prova (1-2) sao valores aceitos. */
static int validar_unidade_prova(int unidade, int prova) {
    if (unidade < 1 || unidade > 4) {
        printf("[ERRO] Unidade invalida: %d. Deve ser entre 1 e 4.\n", unidade);
        return 0;
    }
    if (prova != 1 && prova != 2) {
        printf("[ERRO] Prova invalida: %d. Deve ser 1 ou 2.\n", prova);
        return 0;
    }
    return 1;
}

/* Struct auxiliar que guarda o estado de uma unidade antes de ser alterada.
   E o que fica salvo na Pilha para permitir o Desfazer. */
typedef struct {
    Aluno  *aluno;
    char    materia[50];
    int     unidade;    /* indice 0-based */
    Unidade estado;     /* copia completa antes da edicao */
} SnapshotNota;

/* Cria uma copia do estado atual da unidade e empilha com tipo "nota".
   Deve ser chamada ANTES de qualquer alteracao. */
static int salvar_snapshot_nota(Pilha *p, Aluno *a, char *materia, int unidade_idx) {
    if (!p || !a) return 0;
    Disciplina *d = a->lista_disciplinas;
    while (d) {
        if (strcmp(d->nome, materia) == 0) {
            SnapshotNota *snap = (SnapshotNota*) malloc(sizeof(SnapshotNota));
            if (!snap) return 0;
            snap->aluno   = a;
            snap->unidade = unidade_idx;
            strcpy(snap->materia, materia);
            snap->estado  = d->unidades[unidade_idx];
            salvar_acao(p, "nota", snap);
            return 1;
        }
        d = d->proximo;
    }
    return 0;
}

/* Restaura o estado de uma nota a partir do topo da Pilha (tipo "nota"). */
void desfazer_nota(Pilha *p) {
    if (!p || !p->topo) {
        printf("[INFO] Nada para desfazer.\n");
        return;
    }
    Acao *acao = p->topo;
    if (strcmp(acao->tipo, "nota") != 0) {
        printf("[INFO] A ultima acao na pilha nao e de nota.\n");
        return;
    }
    p->topo = acao->proximo;
    SnapshotNota *snap = (SnapshotNota*) acao->dado;
    Disciplina *d = snap->aluno->lista_disciplinas;
    while (d) {
        if (strcmp(d->nome, snap->materia) == 0) {
            d->unidades[snap->unidade] = snap->estado;
            printf("[UNDO] Restaurado: %s | %s | Unidade %d -> P1:%.2f P2:%.2f Media:%.2f\n",
                   snap->aluno->nome, snap->materia, snap->unidade + 1,
                   snap->estado.prova1, snap->estado.prova2, snap->estado.media_unidade);
            break;
        }
        d = d->proximo;
    }
    free(snap);
    free(acao);
}

/* Lanca nota com validacao completa e salva snapshot para desfazer. */
void lancar_nota_validada(Aluno *lista, Pilha *seguranca,
                          char *mat, char *materia,
                          int unidade, int prova, float nota) {
    if (!validar_nota(nota))                    return;
    if (!validar_unidade_prova(unidade, prova)) return;
    Aluno *a = buscar_aluno(lista, mat);
    if (!a) { printf("[ERRO] Aluno '%s' nao encontrado.\n", mat); return; }
    salvar_snapshot_nota(seguranca, a, materia, unidade - 1);
    lancar_nota(lista, mat, materia, unidade, prova, nota);
    printf("[SUCESSO] Nota %.2f lancada: %s | %s | Unidade %d | Prova %d\n",
           nota, a->nome, materia, unidade, prova);
}

/* Edita uma nota ja existente com validacao e suporte a desfazer. */
void alterar_nota(Aluno *lista, Pilha *seguranca,
                  char *mat, char *materia,
                  int unidade, int prova, float nova_nota) {
    if (!validar_nota(nova_nota))               return;
    if (!validar_unidade_prova(unidade, prova)) return;
    Aluno *a = buscar_aluno(lista, mat);
    if (!a) { printf("[ERRO] Aluno '%s' nao encontrado.\n", mat); return; }
    Disciplina *d = a->lista_disciplinas;
    while (d) {
        if (strcmp(d->nome, materia) == 0) {
            float antiga = (prova == 1) ? d->unidades[unidade-1].prova1
                                        : d->unidades[unidade-1].prova2;
            salvar_snapshot_nota(seguranca, a, materia, unidade - 1);
            if (prova == 1) d->unidades[unidade-1].prova1 = nova_nota;
            else            d->unidades[unidade-1].prova2 = nova_nota;
            d->unidades[unidade-1].media_unidade =
                (d->unidades[unidade-1].prova1 + d->unidades[unidade-1].prova2) / 2.0f;
            printf("[SUCESSO] Nota alterada: %s | %s | U%d P%d: %.2f -> %.2f | Media: %.2f\n",
                   a->nome, materia, unidade, prova, antiga, nova_nota,
                   d->unidades[unidade-1].media_unidade);
            return;
        }
        d = d->proximo;
    }
    printf("[ERRO] Disciplina '%s' nao encontrada.\n", materia);
}

/* Zera uma nota especifica e recalcula a media da unidade. */
void remover_nota(Aluno *lista, Pilha *seguranca,
                  char *mat, char *materia, int unidade, int prova) {
    if (!validar_unidade_prova(unidade, prova)) return;
    Aluno *a = buscar_aluno(lista, mat);
    if (!a) { printf("[ERRO] Aluno '%s' nao encontrado.\n", mat); return; }
    Disciplina *d = a->lista_disciplinas;
    while (d) {
        if (strcmp(d->nome, materia) == 0) {
            salvar_snapshot_nota(seguranca, a, materia, unidade - 1);
            if (prova == 1) d->unidades[unidade-1].prova1 = 0.0f;
            else            d->unidades[unidade-1].prova2 = 0.0f;
            d->unidades[unidade-1].media_unidade =
                (d->unidades[unidade-1].prova1 + d->unidades[unidade-1].prova2) / 2.0f;
            printf("[SUCESSO] Nota zerada: %s | %s | Unidade %d | Prova %d\n",
                   a->nome, materia, unidade, prova);
            return;
        }
        d = d->proximo;
    }
    printf("[ERRO] Disciplina '%s' nao encontrada.\n", materia);
}

/* Exibe o quadro completo de notas de um aluno (todas as disciplinas). */
void consultar_notas_aluno(Aluno *lista, char *mat) {
    Aluno *a = buscar_aluno(lista, mat);
    if (!a) { printf("[ERRO] Aluno '%s' nao encontrado.\n", mat); return; }
    printf("\n======== QUADRO DE NOTAS: %s (%s) ========\n", a->nome, a->matricula);
    Disciplina *d = a->lista_disciplinas;
    while (d) {
        printf("%-15s | ", d->nome);
        for (int i = 0; i < 4; i++) {
            printf("U%d[P1:%.1f P2:%.1f M:%.1f] ",
                   i+1, d->unidades[i].prova1,
                   d->unidades[i].prova2, d->unidades[i].media_unidade);
        }
        printf("\n");
        d = d->proximo;
    }
    printf("===================================================\n");
}

/* Calcula a media geral do aluno (media das medias_final de cada disciplina).
   Atualiza o campo media_final de cada Disciplina como efeito colateral. */
float calcular_media_aluno(Aluno *a) {
    if (!a) return 0.0f;
    float soma = 0.0f;
    int   qtd  = 0;
    Disciplina *d = a->lista_disciplinas;
    while (d) {
        float su = 0.0f;
        for (int i = 0; i < 4; i++) su += d->unidades[i].media_unidade;
        d->media_final = su / 4.0f;
        soma += d->media_final;
        qtd++;
        d = d->proximo;
    }
    return (qtd > 0) ? (soma / (float)qtd) : 0.0f;
}

/* Fechamento da turma: percorre a lista, calcula medias e exibe
   relatorio com Aprovados (media >= 5.0) e Reprovados. */
void gerar_relatorio_final(Turma *t) {
    if (!t) { printf("[ERRO] Nenhuma turma disponivel.\n"); return; }
    if (!t->lista_alunos) {
        printf("[AVISO] Turma %s sem alunos matriculados.\n", t->codigo);
        return;
    }
    int aprovados = 0, reprovados = 0;
    printf("\n##############################################################\n");
    printf("##    FECHAMENTO DE NOTAS — TURMA %-10s             ##\n", t->codigo);
    printf("##    Serie: %d | Alunos: %d / %d vagas                  ##\n",
           t->serie, t->qtd_atual, t->limite_vagas);
    printf("##############################################################\n");
    Aluno *atual = t->lista_alunos;
    while (atual != NULL) {
        float mg = calcular_media_aluno(atual);
        const char *status = (mg >= 5.0f) ? "APROVADO " : "REPROVADO";
        if (mg >= 5.0f) aprovados++; else reprovados++;
        printf("\n>> %-20s | Mat: %-12s | Media: %5.2f | [%s]\n",
               atual->nome, atual->matricula, mg, status);
        Disciplina *d = atual->lista_disciplinas;
        while (d) {
            printf("   %-15s | ", d->nome);
            for (int i = 0; i < 4; i++)
                printf("U%d:%.1f ", i+1, d->unidades[i].media_unidade);
            printf("| Anual: %.2f", d->media_final);
            if (d->media_final < 5.0f) printf(" [!]");
            printf("\n");
            d = d->proximo;
        }
        printf("   ----------------------------------------------------------\n");
        atual = atual->proximo;
    }
    printf("\n##############################################################\n");
    printf("##  Aprovados: %d | Reprovados: %d | Total: %d             ##\n",
           aprovados, reprovados, aprovados + reprovados);
    if (aprovados + reprovados > 0)
        printf("##  Taxa de aprovacao: %.1f%%                              ##\n",
               100.0f * aprovados / (aprovados + reprovados));
    printf("##############################################################\n\n");
}
