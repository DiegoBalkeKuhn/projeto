import utilitarios.Teclado2;

public class testarSePassou {
    public static void main(String[] args) {
        String nome = Teclado2.read("Digite o nome do aluno:", String.class);
        int idade = Teclado2.read("Digite a idade do aluno:", Integer.class);
        String curso = Teclado2.read("Digite o curso do aluno:", String.class);
        double notaMedia = Teclado2.read("Digite a nota média do aluno:", Double.class);

        Aluno aluno1 = new Aluno(nome, idade, curso, notaMedia);

        if (aluno1.notaMedia >= 7) {
            System.out.println("Parabéns " + aluno1.nome + ", você foi aprovado!");
        } else {
            System.out.println("Infelizmente " + aluno1.nome + ", você não foi aprovado.");
        }
    }
}
