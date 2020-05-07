package exception;

public class WrongFormulaException extends Exception{

    public WrongFormulaException(String message) {
        super(message);
    }

    public WrongFormulaException(String message, Throwable cause) {
        super(message, cause);
    }

}
