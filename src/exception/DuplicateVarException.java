package exception;

public class DuplicateVarException extends Exception{

    public DuplicateVarException(String message) {
        super(message);
    }

    public DuplicateVarException(String message, Throwable cause) {
        super(message, cause);
    }

}
