# Projeto-de-ED
🏛️ Resumo da Arquitetura: Sistema Escolar Kolping
1. Identidade e Escopo
Nome da Instituição: Definimos o nome como Escola Kolping, estabelecendo uma identidade visual nos e-mails institucionais e nos cabeçalhos de relatórios.

Objetivo: Criar uma infraestrutura robusta em C para gerenciar o fluxo administrativo de alunos, professores e turmas.

2. Estrutura de Dados (Modelagem)
Implementamos um sistema de Listas Encadeadas Dinâmicas para garantir que a memória seja usada de forma eficiente:

Lista de Alunos: Cada nó contém dados pessoais, matrícula única e um ponteiro para sua grade curricular específica.

Lista de Disciplinas: Cada aluno possui sua própria lista de matérias, conectada ao seu cadastro.

Lista de Professores: Uma lista global que armazena todos os docentes da instituição, permitindo que eles sejam vinculados a múltiplas disciplinas.

Estrutura de Turmas: Um container que organiza a lotação física (vagas) e associa alunos a um professor regente.

3. Regras de Negócio e Automação
Grades Predefinidas por Ano: Criamos uma lógica automática onde o sistema identifica a série do aluno e aloca as matérias corretas:

Fundamental (Ex: 6º ano): 8 disciplinas (Português, Matemática, Ciências, etc.).

Ensino Médio (Ex: 1º ano/Série 10): 10 disciplinas (Incluindo Física, Química, Filosofia e Sociologia).

Geração de Identidade Digital: O sistema gera automaticamente e-mails acadêmicos e funcionais baseados no nome e ID do usuário.

4. Segurança e Robustez (O Diferencial do Engenheiro)
Controle de Vagas: Implementamos uma trava que impede a matrícula em turmas que já atingiram o limite máximo.

Gestão de Memória: Criamos funções de exclusão em cascata (deletar aluno remove também suas notas e matérias).

Prevenção de Erros (Dangling Pointers): Desenvolvemos uma função que, ao remover um professor, limpa automaticamente todos os vínculos dele nos boletins dos alunos, evitando que o sistema tente ler memórias inexistentes.
