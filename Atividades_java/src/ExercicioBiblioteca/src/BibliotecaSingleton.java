import java.util.ArrayList;

public class BibliotecaSingleton {

    //Para o padrão Singleton
    private static BibliotecaSingleton unicaInstancia; 
    private static ArrayList<Livro> acervoLivros;
    //PREENCER: Como seria o armazenamento dos Alunos cadastrados na Biblioteca?

    //Forçando o construtor privado, para garantir o padrão Singleton e nenhuma nova instância
    private BibliotecaSingleton() { 
        acervoLivros = new ArrayList<Livro>();
    }

    public static BibliotecaSingleton getInstanciaSingleton(){
        if (unicaInstancia == null) {
            unicaInstancia = new BibliotecaSingleton();
        }
        return unicaInstancia;
    }

    public static boolean inserirNovoLivro (String EAN, String titulo, String autor){

        Livro livro = new Livro(EAN, titulo, autor);
        acervoLivros.add(livro); 
        
        return true; //deu tupo certo

    }

    public void listarAcervo (){

        //evitando a lista vazia
        if (acervoLivros != null ){ 
            System.out.println();
            //Para cada objeto Livro como sendo "livro" de Livros
            for (Livro livro : acervoLivros ){ 
                System.out.println(livro.toString());
            }
        } else {
            System.out.println("Acervo vazio.");
        }
    }

    public Livro localizarLivro (String isbn){

        Livro achei = null;
        //cortando a execução de cara
        if (acervoLivros == null) return null; 

        //Para cada objeto Livro iterado sendo "livro" de acervoLivros
        for (Livro livro : acervoLivros){ 
            /*if (?) {//fazer busca para localizar livro com for each e break quando achar, usar objeto achei
                achei = livro;
                break;
            /* */
        }
        //redundante abaixo, proposital pra entendimento ao invés de fazer um return direto.        
        if (achei != null){
            return achei;
        } else {
            return null;
        }
    }

}
