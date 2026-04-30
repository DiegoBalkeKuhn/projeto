package MAV-2.0.exemplos;

public class Exemplo1 {

    private int atributo;

        public Exemplo1(){ //reescrevendo construtor padrão
        //algo?
    }

    public Exemplo1 (int atributo){
        this(); //chamando construtor padrão
        setAtributo(atributo); 
    }

    public int getAtributo() {
        return atributo;
    }

    public void setAtributo(int atributo) {
        //validação? Exceção? 
        this.atributo = atributo;
    }

    public void procedimentoQualquer(){
        //Este método vai fazer algo, sem parâmetros
    }
    
    public void procedimentoQualquer(int comParametro){ //sobrecarga
        //Este método vai fazer algo, com parâmetro
    }
  
    //@override
    public String toString(){
        //super.toString(); //Executa o pai?
        return ("Valor interno: " + atributo);
    }

    private int metodoQualquerInterno(int temParametro){ //privado da classe
        int algo = 0;
        algo = algo + temParametro;
        return algo;
    }

    

}
