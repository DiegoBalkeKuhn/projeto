import utilitarios.*;

public class App {

    static BibliotecaSingleton nossaBiblioteca;

    private static void inicializarSistema(){

        //Fazendo o uso do padrão Singleton de única instância
        nossaBiblioteca = BibliotecaSingleton.getInstanciaSingleton();

        Livro livro1, livro2, livro3;

        livro1 = new Livro ("111111", "Apostila OO", "Valdemar Lorenzon Junior");        
        livro2 = new Livro ("112112", "Apostila de Exercícios OO", "José Toniazzo");

        //PREENCHER: livro3 = clone do livro 1, ver método na classe
        
    }

    private static void listaOpcoes(){
        //quais opcoes?
        System.out.println("\n1-Listar Acervo.");
        System.out.println("2-Emprestimo.");
        System.out.println("3-Devolucao.");    
        System.out.println("4-Localizar Livro.");
        System.out.println("5-Localizar Aluno.");
        System.out.println("6-Cadastrar Livro.");
        System.out.println("5-Cadastrar Aluno.");
        System.out.println("0-Sair.\n");
    }


    public static void main(String[] args) throws Exception {

        int opcaoMenu;

        inicializarSistema(); 

        do{
            listaOpcoes();
            opcaoMenu = Teclado.readInt("Informe a opção:");

            switch (opcaoMenu) {                
                
                case 0:
                    System.out.println("Fim de execução!");
                    break;

                case 1:
                    nossaBiblioteca.listarAcervo();
                    break;
                    
                //demais opções?
                //Como realizar o emprestimo: Como resolver o objeto aluno?
                //Como realizar a devolucao?

                case 4:
                    Livro livroLocalizado;
                    livroLocalizado  = nossaBiblioteca.localizarLivro(Teclado.readString("Informe o isbn:"));
                    if (livroLocalizado == null){
                        System.err.println("Livro não localizado no acervo!");
                    } else {
                        //indicar se livro disponivel                        
                    }
                    break;            
                default:
                    break;
            }
        } while (opcaoMenu != 0);                

    }
}
