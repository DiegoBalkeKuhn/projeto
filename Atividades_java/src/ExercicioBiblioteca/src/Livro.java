public class Livro {

    private final int ESTOQUE = 1;
    private String isbn;
    private String titulo;
    private String autor;    
    //PREENCHER: Criar um atributo que representa o status do livro, está disponível ou não?
    //PREENCHER: Criar um atributo que representa qual aluno detem o emprestimo do livro? Associação com quem? è sempre um Aluno apenas.
    

    public Livro(String isbn, String titulo, String autor) {
        //PREENCER: Ajustar a classe com métodos get e set e arrumar o construtor
        this.isbn = isbn;
        this.titulo = titulo;
        this.autor = autor;
        ////PREENCER: compreender os demais atributos indicados
    }

    ////PREENCER: Completar e verificar uma análise de ajuste para getteres e setters publicos ou privados?

    public String getTitulo() {
        return titulo;
    }    

    public String getAutor() {
        return autor;
    }

    public void setTitulo(String titulo) {
        this.titulo = titulo;
    }

    public void setAutor(String autor) {
        this.autor = autor;
    }

    //PREENCER: Fazer override de toString para ter um retorno válido de dados como String do livro   
    @Override
    public String toString() {
        return "Livro{" + "isbn=" + isbn + ", titulo=" + titulo + ", autor=" + autor + '}';
    }
    
    //PREENCER: criar possibilidade de retornar um clone do livro, já que se pode ter mais do que um exemplar do mesmo
    public Livro clone() {
        return new Livro(this.isbn, this.titulo, this.autor);
    }
    //Nesta caso não seguir qualquer ideia já pronta, fazer um clone no braço do this para um novo objeto:

    //Duplicar o objeto
        
}