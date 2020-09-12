import { success, failure } from "./parse-result-creators";
import { parsePlus, parseInteger } from "./parsers";
import { reportSuccess, reportFailure } from "./reporters";

export const parse = (input: string): string => {
  const result1 = parseInteger(input);
  if (result1.kind === "failure") {
    return reportFailure(result1);
  }
  const { data: int1, rest: input1 } = result1;

  const result2 = parsePlus(input1);
  if (result2.kind === "failure") {
    return reportFailure(result2);
  }
  const { rest: input2 } = result2;

  const result3 = parseInteger(input2);
  if (result3.kind === "failure") {
    return reportFailure(result3);
  }
  const { data: int2, rest: input3 } = result3;

  if (input3.length > 0) {
    return reportFailure(failure("end of input", input3));
  }

  return reportSuccess(success(int1 + int2, input3));
};
